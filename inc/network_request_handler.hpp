#ifndef NETWORK_REQUEST_HANDLER_HPP
#define NETWORK_REQUEST_HANDLER_HPP

#include "tcp_request.hpp"

#include <set>
#include <list>
#include <memory>
#include <utility>
#include <variant>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <stdexcept>
#include <ranges>

namespace tristan::log {
    class Log;
}  // namespace tristan::log

namespace tristan::network {

    namespace private_ {
        class AsyncRequestHandler;
    }  // namespace private_

       //    using SuppoertedRequestTypes = std::variant< std::shared_ptr< TcpRequest >, std::shared_ptr< HttpRequest > >;

    /**
     * \class NetworkRequestsHandler
     * \brief Implements network requests queue.
     * \Threadsafe Yes
     */
    class NetworkRequestsHandler {

        NetworkRequestsHandler() = default;

        static auto instance() -> NetworkRequestsHandler&;

    public:
        NetworkRequestsHandler(const NetworkRequestsHandler& p_other) = delete;

        NetworkRequestsHandler(NetworkRequestsHandler&& p_other) = delete;

        NetworkRequestsHandler& operator=(const NetworkRequestsHandler& p_other) = delete;

        NetworkRequestsHandler& operator=(NetworkRequestsHandler&& p_other) = delete;

        ~NetworkRequestsHandler();

        /**
         * Sets user provided logger.
         * \param p_log std::unique_ptr<tristan::log::Log>&&
         */
        static void setLogger(std::unique_ptr< tristan::log::Log >&& p_log);

        /**
         * \brief Sets simultaneous requests limit which by default is 5.
         * \param p_limit uint8_t.
         */
        static void setActiveDownloadsLimit(uint8_t p_limit);

        /**
         * \brief Starts the handler loop.
         * \note This function if blocking and should be ran in a separate thread.
         */
        static void run();

        /**
         * \brief Stops the processing of all network requests.
         * \note This function does not clear any data.
         */
        static void stop();

        /**
         * \brief Pauses the processing of the requests.
         * The difference with stop() call is that in this case handler doesn't exists from execution loop.
         */
        static void pause();

        /**
         * \brief Resumes the processing of the requests.
         */
        static void resume();

        /**
         * \brief Registers callback functions which will be invoked when request processing is stopped.
         * \param p_function std::function<void()>&& functor
         */
        static void notifyWhenExit(std::function< void() >&& p_function);

        /**
         * \brief Registers callback functions which will be invoked when request processing is stopped.
         * \tparam Object Type which holds the function member to invoke.
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)()
         */
        template < class Object > static void notifyWhenExit(std::weak_ptr< Object > p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when reqeust processing is stopped.
         * \tparam Object Type which holds the function member to invoke.
         * \param p_object Object*
         * \param p_function void (Object::*functor)()
         */
        template < class Object > static void notifyWhenExit(Object* p_object, void (Object::*p_function)());

        /**
         * \brief Adds request to the queue
         * \param p_request std::shared_ptr<Request>
         */
        static void addRequest(std::shared_ptr< NetworkRequestBase >&& p_request);

        static auto pendingRequests() -> std::ranges::ref_view< std::multiset< std::shared_ptr< NetworkRequestBase > > >;

        /**
         * \brief Returns list of currently active requests.
         * \return std::list<std::shared_ptr<Request>>
         */
        static auto activeRequests() -> std::list< std::shared_ptr< NetworkRequestBase > >&;

        /**
         * \brief Returns queue of requests which encountered error.
         * \return const std::queue<std::shared_ptr<Request>>&
         */
        static auto errorRequests() -> const std::list< std::shared_ptr< NetworkRequestBase > >&;

    protected:
    private:
        std::mutex m_nr_multiset_lock;
        std::mutex m_error_nr_lock;
        std::mutex m_active_nr_lock;

//        struct Compare {
//            bool operator()(const std::shared_ptr< NetworkRequestBase >& p_left, const std::shared_ptr< NetworkRequestBase >& p_right) const {
//                return p_left < p_right;
//            }
//        };

        std::multiset< std::shared_ptr< NetworkRequestBase > > m_requests;

        std::list< std::shared_ptr< NetworkRequestBase > > m_error_requests;
        std::list< std::shared_ptr< NetworkRequestBase > > m_active_requests;

        std::vector< std::function< void() > > m_notify_when_exit_functions;

        std::unique_ptr< private_::AsyncRequestHandler > m_async_tcp_requests_handler;
        std::unique_ptr< private_::SyncNetworkRequestHandlerImpl > m_request_handler;
        std::thread m_async_request_handler_thread;

        std::atomic< bool > m_working;
        std::atomic< bool > m_paused;

        void _run();

        void _pause();

        void _resume();

        void _stop();

        void _addRequest(std::shared_ptr< NetworkRequestBase >&& request);

        template < class Object > void _notifyWhenExit(std::weak_ptr< Object > p_object, void (Object::*p_function)());

        template < class Object > void _notifyWhenExit(Object* p_object, void (Object::*p_function)());

        void _notifyWhenExit(std::function< void() >&& p_function);
    };

    template < class Object > void NetworkRequestsHandler::notifyWhenExit(std::weak_ptr< Object > p_object, void (Object::*p_function)()) {
        NetworkRequestsHandler::instance()._notifyWhenExit(p_object, p_function);
    }

    template < class Object > void NetworkRequestsHandler::notifyWhenExit(Object* p_object, void (Object::*p_function)()) {
        NetworkRequestsHandler::instance()._notifyWhenExit(p_object, p_function);
    }

    template < class Object > void NetworkRequestsHandler::_notifyWhenExit(std::weak_ptr< Object > p_object, void (Object::*p_function)()) {
        m_notify_when_exit_functions.emplace_back([p_object, p_function]() -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object);
            }
        });
    }

    template < class Object > void tristan::network::NetworkRequestsHandler::_notifyWhenExit(Object* p_object, void (Object::*p_function)()) {
        m_notify_when_exit_functions.emplace_back([p_object, p_function]() -> void {
            std::invoke(p_function, p_object);
        });
    }
}  // namespace tristan::network

#endif  // NETWORK_REQUEST_HANDLER_HPP
