#ifndef NETWORK_REQUEST_HANDLER_HPP
#define NETWORK_REQUEST_HANDLER_HPP

#include "network_request.hpp"

#include <queue>
#include <list>
#include <memory>
#include <utility>
#include <variant>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <concepts>

template <class Request> concept hasDoRequestMember = requires(const Request& request){
    request.doRequest();
};
template <class Request> concept isComparable = requires(const Request& left, const Request& right){
    left < right;
};

namespace tristan::network{
    /**
     * \class NetworkRequestsHandler
     * \brief Implements http network request queue.
     * \Threadsafe Yes
     */
    template <class Request, class Response> requires hasDoRequestMember<Request> && isComparable<Request> class NetworkRequestsHandler{
        /**
         * \brief Constructor. Sets active limit to 5.
         *
         */
        NetworkRequestsHandler():
                m_active_requests_limit(5),
                m_active_requests(0),
                m_working(true),
                m_paused(false){}

        template<Request, Response> static auto instance() -> NetworkRequestsHandler<Request, Response>&{
            static NetworkRequestsHandler<Request, Response> network_requests_handler;

            return network_requests_handler;
        }
      public:
        NetworkRequestsHandler(const NetworkRequestsHandler& other) = delete;
        NetworkRequestsHandler(NetworkRequestsHandler&& other) = delete;

        NetworkRequestsHandler& operator=(const NetworkRequestsHandler& other) = delete;
        NetworkRequestsHandler& operator=(NetworkRequestsHandler&& other) = delete;

        ~NetworkRequestsHandler() = default;

        /**
         * \brief Sets simultaneous requests limit which by default is 5.
         * \param limit uint8_t.
         */
        inline static void setActiveRequestsLimit(uint8_t limit){ NetworkRequestsHandler::instance<Request, Response>()._setActiveRequestsLimit(limit); }

        /**
         * \brief Starts the handler loop.
         * \note This function if blocking and should be ran in a separate thread.
         */
        inline static void run(){ NetworkRequestsHandler::instance<Request, Response>()._run(); }

        /**
         * \brief Stops the processing of all network requests.
         * \note This function does not clear any data.
         */
        inline static void stop(){ NetworkRequestsHandler::instance<Request, Response>()._stop(); }

        /**
         * \brief Pauses the processing of the requests.
         * The difference with stop() call is that in this case handler doesn't exists from execution loop.
         */
        inline static void pause(){ NetworkRequestsHandler::instance<Request, Response>().m_paused.store(true, std::memory_order_relaxed); }
        /**
         * \brief Resumes the processing of the requests.
         */
        inline static void resume(){ NetworkRequestsHandler::instance<Request, Response>().m_paused.store(false, std::memory_order_relaxed); }
        /**
         * \brief Registers callback functions which will be invoked when request processing is stopped.
         * \param functor std::function<void()> functor
         */
        inline static void notifyWhenExit(std::function<void()> functor){ NetworkRequestsHandler::instance<Request, Response>()._notifyWhenExit(functor); }

        /**
         * \brief Registers callback functions which will be invoked when reqeust processing is stopped.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template<class Object> inline static void notifyWhenExit(std::weak_ptr<Object> object, void (Object::*functor)()){ NetworkRequestsHandler::instance<Request, Response>()._notifyWhenExit(object, functor); }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when reqeust processing is stopped.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template<class Object> inline static void notifyWhenExit(Object* object, void (Object::*functor)()){ NetworkRequestsHandler::instance<Request, Response>()._notifyWhenExit(object, functor); }

        /**
         * \brief Adds request to the queue
         * \param request std::shared_ptr<Request>
         */
        inline static void addRequest(std::shared_ptr<Request> request){ NetworkRequestsHandler::instance<Request, Response>()._addRequest(std::move(request)); }

        /**
         * \brief Returns list of currently active requests.
         * \return std::list<std::shared_ptr<Request>>
         */
        inline static auto activeRequests() -> std::list<std::shared_ptr<Request>>&{ return NetworkRequestsHandler::instance<Request, Response>().m_active_requests; }

        /**
         * \brief Returns queue of requests which encountered error.
         * \return const std::queue<std::shared_ptr<Request>>&
         */
        inline static auto errorRequests() -> const std::list<std::shared_ptr<Request>>&{ return NetworkRequestsHandler::instance<Request, Response>().m_error_requests; }

      protected:

      private:

        std::mutex m_nr_queue_lock;
        std::mutex m_error_nr_lock;
        std::mutex m_active_nr_lock;

        struct Compare{
            bool operator()(const std::shared_ptr<Request>& left, const std::shared_ptr<Request>& right) const{
                return *left < *right;
            }
        };

        std::priority_queue<std::shared_ptr<Request>, std::vector<std::shared_ptr<Request>>, Compare()> m_requests;

        std::queue<std::shared_ptr<Request>> m_error_requests;
        std::list<std::shared_ptr<Request>> m_active_requests;

        std::vector<std::function<void()>> m_notify_when_exit_functors;

        uint8_t m_active_requests_limit;
        std::atomic<bool> m_working;
        std::atomic<bool> m_paused;

        void _setActiveRequestsLimit(uint8_t limit){ m_active_requests_limit = limit; }

        void _run(){
            if (m_working.load(std::memory_order_relaxed)){
                return;
            }
            while (m_working.load(std::memory_order_relaxed)){
                if (m_requests.empty() || m_active_requests.size() >= m_active_requests_limit || m_paused.load(std::memory_order_relaxed)){
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
                else{
                    std::unique_lock<std::mutex> lock(m_nr_queue_lock);
                    auto request = m_requests.top();
                    m_requests.pop();
                    lock.unlock();
                    request->notifyWhenFinished([this](std::shared_ptr<Response> response) -> void{
                        std::scoped_lock<std::mutex> lock(m_active_nr_lock);
                        m_active_requests.remove_if([response](std::shared_ptr<Request> stored_request){
                            return stored_request->uuid() == response->uuid();
                        });
                    });
                    request->notifyWhenError([this](const std::pair<std::string, std::error_code>& error){
                        for (auto&& l_request : m_active_requests){
                            if (l_request->uuid() == error.first){
                                std::scoped_lock<std::mutex> lock(m_error_nr_lock);
                                m_error_requests.push(l_request);
                            }
                        }
                        std::scoped_lock<std::mutex> lock(m_active_nr_lock);
                        m_active_requests.remove_if([error](const std::shared_ptr<Request>& stored_request){
                            return stored_request->uuid() == error.first;
                        });
                    });
                    request->notifyWhenPaused([this]() -> void{
                        ++m_active_requests_limit;
                    });
                    request->notifyWhenResumed([this]() -> void{
                        --m_active_requests_limit;
                    });
                    std::thread(&Request::doRequest, request).detach();
                    std::scoped_lock<std::mutex> active_lock(m_active_nr_lock);
                    m_active_requests.push_back(request);
                }
            }
            if (!m_working.load(std::memory_order_relaxed)){
                if (!m_notify_when_exit_functors.empty()){
                    for (const auto& functor: m_notify_when_exit_functors){
                        functor();
                    }
                }
            }
        }
        void _stop(){
            m_working.store(false, std::memory_order_relaxed);
        }

        void _addRequest(std::shared_ptr<Request> request){
            std::scoped_lock<std::mutex> lock(m_nr_queue_lock);
            m_requests.push(request);
        }

        template<class Object> void _notifyWhenExit(std::weak_ptr<Object> object, void (Object::*functor)()){
            m_notify_when_exit_functors.emplace_back([object, functor]() -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object);
                }
            });
        }

        template<class Object> void _notifyWhenExit(Object* object, void (Object::*functor)()){
            m_notify_when_exit_functors.emplace_back([object, functor]() -> void{
                std::invoke(functor, object);
            });
        }

        void _notifyWhenExit(std::function<void()> functor){
            m_notify_when_exit_functors.emplace_back(functor);
        }
    };

} //End of tristan::network namespace

#endif //NETWORK _REQUEST_HANDLER_HPP
