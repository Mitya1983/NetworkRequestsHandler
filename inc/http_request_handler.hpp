#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include "http_request.hpp"

#include <queue>
#include <list>
#include <memory>
#include <utility>
#include <variant>
#include <mutex>
#include <thread>
#include <atomic>

namespace tristan::network{
    /**
     * \class HttpRequestsHandler
     * \brief Implements http network request queue.
     * \Threadsafe Yes
     */
    class HttpRequestsHandler{
        /**
         * \brief Constructor. Sets active limit to 5.
         *
         */
        HttpRequestsHandler();
        static auto instance() -> HttpRequestsHandler&;
      public:
        HttpRequestsHandler(const HttpRequestsHandler& other) = delete;
        HttpRequestsHandler(HttpRequestsHandler&& other) = delete;

        HttpRequestsHandler& operator=(const HttpRequestsHandler& other) = delete;
        HttpRequestsHandler& operator=(HttpRequestsHandler&& other) = delete;

        ~HttpRequestsHandler() = default;
        /**
         * \brief Sets simultaneous requests limit which by default is 5.
         * \param limit uint8_t.
         */
        inline static void setActiveRequestsLimit(uint8_t limit){ HttpRequestsHandler::instance()._setActiveRequestsLimit(limit); }
        /**
         * \brief Starts the handler loop.
         * \note This function if blocking and should be ran in a separate thread.
         */
        inline static void run(){ HttpRequestsHandler::instance()._run(); }
        /**
         * \brief Stops the processing of all network requests.
         * \note This function does not clear any data.
         */
        inline static void stop(){ HttpRequestsHandler::instance()._stop(); }
        /**
         * \brief Registers callback functions which will be invoked when reqeust processing is stopped.
         * \param functor std::function<void()> functor
         */
        inline static void notifyWhenExit(std::function<void()> functor){ HttpRequestsHandler::instance()._notifyWhenExit(functor); }
        /**
         * \brief Registers callback functions which will be invoked when reqeust processing is stopped.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template<class Object> inline static void notifyWhenExit(std::weak_ptr<Object> object, void (Object::*functor)()){ HttpRequestsHandler::instance()._notifyWhenExit(object, functor); }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when reqeust processing is stopped.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template<class Object> inline static void notifyWhenExit(Object* object, void (Object::*functor)()){ HttpRequestsHandler::instance()._notifyWhenExit(object, functor); }
        /**
         * \brief Adds request to the queue
         * \param request std::shared_ptr<HttpRequest>
         */
        inline static void addRequest(std::shared_ptr<HttpRequest> request){ HttpRequestsHandler::instance()._addRequest(std::move(request)); }
        /**
         * \brief Returns list of currently active requests.
         * \return std::list<std::shared_ptr<HttpRequest>>
         */
        inline static auto activeRequests() -> std::list<std::shared_ptr<HttpRequest>>&{ return HttpRequestsHandler::instance().m_active_requests; }
        /**
         * \brief Returns list of high priority requests which are in the queue and which are not being processed yet.
         * \return const std::queue<std::shared_ptr<HttpRequest>>&
         */
        inline static auto highPriorityRequests() -> const std::queue<std::shared_ptr<HttpRequest>>& {return HttpRequestsHandler::instance().m_high_priority_requests;}
        /**
         * \brief Returns list of normal priority requests which are in the queue and which are not being processed yet.
         * \return const std::queue<std::shared_ptr<HttpRequest>>&
         */
        inline static auto normalPriorityRequests() -> const std::queue<std::shared_ptr<HttpRequest>>& {return HttpRequestsHandler::instance().m_normal_priority_requests;}
        /**
         * \brief Returns list of low priority requests which are in the queue and which are not being processed yet.
         * \return const std::queue<std::shared_ptr<HttpRequest>>&
         */
        inline static auto lowPriorityRequests() -> const std::queue<std::shared_ptr<HttpRequest>>& {return HttpRequestsHandler::instance().m_low_priority_requests;}

      protected:

      private:
        std::mutex m_lock;

        std::queue<std::shared_ptr<HttpRequest>> m_high_priority_requests;
        std::queue<std::shared_ptr<HttpRequest>> m_normal_priority_requests;
        std::queue<std::shared_ptr<HttpRequest>> m_low_priority_requests;
        std::list<std::shared_ptr<HttpRequest>> m_active_requests;

        std::vector<std::function<void()>> m_notify_when_exit_functors;

        uint8_t m_active_requests_limit;
        std::atomic<bool> m_working;

        void _setActiveRequestsLimit(uint8_t limit){ m_active_requests_limit = limit; }

        void _run();
        void _stop();

        void _addRequest(std::shared_ptr<HttpRequest> request);

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

#endif //HTTP_REQUEST_HANDLER_HPP
