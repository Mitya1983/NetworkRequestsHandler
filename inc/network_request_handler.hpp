#ifndef NETWORK_REQUEST_HANDLER_HPP
#define NETWORK_REQUEST_HANDLER_HPP

#include "tcp_request.hpp"
#include "async_request_handler.hpp"
#include "sync_network_request_handler_impl.hpp"

#include <queue>
#include <list>
#include <memory>
#include <utility>
#include <variant>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <stdexcept>

namespace tristan::network {

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
        NetworkRequestsHandler(const NetworkRequestsHandler& other) = delete;

        NetworkRequestsHandler(NetworkRequestsHandler&& other) = delete;

        NetworkRequestsHandler& operator=(const NetworkRequestsHandler& other) = delete;

        NetworkRequestsHandler& operator=(NetworkRequestsHandler&& other) = delete;

        ~NetworkRequestsHandler();

        /**
         * \brief Sets simultaneous requests limit which by default is 5.
         * \param limit uint8_t.
         */
        inline static void setActiveDownloadsLimit(uint8_t limit) {
            NetworkRequestsHandler::instance().m_async_tcp_requests_handler->setMaxDownloadsCount(limit);
        }

        /**
         * \brief Starts the handler loop.
         * \note This function if blocking and should be ran in a separate thread.
         */
        inline static void run() { NetworkRequestsHandler::instance()._run(); }

        /**
         * \brief Stops the processing of all network requests.
         * \note This function does not clear any data.
         */
        inline static void stop() { NetworkRequestsHandler::instance()._stop(); }

        /**
         * \brief Pauses the processing of the requests.
         * The difference with stop() call is that in this case handler doesn't exists from execution loop.
         */
        inline static void pause() {
            NetworkRequestsHandler::instance()._pause();
        }

        /**
         * \brief Resumes the processing of the requests.
         */
        inline static void resume() {
            NetworkRequestsHandler::instance()._resume();
        }

        /**
         * \brief Registers callback functions which will be invoked when request processing is stopped.
         * \param functor std::function<void()>&& functor
         */
        inline static void notifyWhenExit(std::function< void() >&& functor) {
            NetworkRequestsHandler::instance()._notifyWhenExit(std::move(functor));
        }

        /**
         * \brief Registers callback functions which will be invoked when request processing is stopped.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template < class Object >
        inline static void notifyWhenExit(std::weak_ptr< Object > object, void (Object::*functor)()) {
            NetworkRequestsHandler::instance()._notifyWhenExit(object, functor);
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when reqeust processing is stopped.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template < class Object >
        inline static void notifyWhenExit(Object* object, void (Object::*functor)()) {
            NetworkRequestsHandler::instance()._notifyWhenExit(object, functor);
        }

        /**
         * \brief Adds request to the queue
         * \param request std::shared_ptr<Request>
         */
        static void addRequest(std::shared_ptr< NetworkRequestBase >&& request);

        /**
         * \brief Returns list of currently active requests.
         * \return std::list<std::shared_ptr<Request>>
         */
        inline static auto activeRequests() -> std::list< std::shared_ptr< NetworkRequestBase > >& {
            return NetworkRequestsHandler::instance().m_active_requests;
        }

        /**
         * \brief Returns queue of requests which encountered error.
         * \return const std::queue<std::shared_ptr<Request>>&
         */
        inline static auto errorRequests() -> const std::list< std::shared_ptr< NetworkRequestBase > >& {
            return NetworkRequestsHandler::instance().m_error_requests;
        }

    protected:
    private:
        std::mutex m_nr_queue_lock;
        std::mutex m_error_nr_lock;
        std::mutex m_active_nr_lock;

        struct Compare {
            bool operator()(const std::shared_ptr< NetworkRequestBase >& left, const std::shared_ptr< NetworkRequestBase >& right) const {
                return left < right;
            }
        };

        std::priority_queue< std::shared_ptr< NetworkRequestBase >, std::deque< std::shared_ptr< NetworkRequestBase > >, Compare >
            m_requests;

        std::list< std::shared_ptr< NetworkRequestBase > > m_error_requests;
        std::list< std::shared_ptr< NetworkRequestBase > > m_active_requests;

        std::vector< std::function< void() > > m_notify_when_exit_functors;

        std::unique_ptr< AsyncRequestHandler > m_async_tcp_requests_handler;
        std::unique_ptr< private_::SyncNetworkRequestHandlerImpl > m_request_handler;
        std::thread m_async_request_handler_thread;

        std::atomic< bool > m_working;
        std::atomic< bool > m_paused;

        void _run();

        void _pause();

        void _resume();

        void _stop();

        void _addRequest(std::shared_ptr< NetworkRequestBase >&& request);

        template < class Object >
        void _notifyWhenExit(std::weak_ptr< Object > object, void (Object::*functor)()) {
            m_notify_when_exit_functors.emplace_back([object, functor]() -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object);
                }
            });
        }

        template < class Object > void _notifyWhenExit(Object* object, void (Object::*functor)()) {
            m_notify_when_exit_functors.emplace_back([object, functor]() -> void {
                std::invoke(functor, object);
            });
        }

        void _notifyWhenExit(std::function< void() >&& functor) {
            m_notify_when_exit_functors.emplace_back(functor);
        }
    };
}  // namespace tristan::network

#endif  // NETWORK_REQUEST_HANDLER_HPP
