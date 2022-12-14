#ifndef ASYNC_REQUEST_HANDLER_HPP
#define ASYNC_REQUEST_HANDLER_HPP

#include "async_network_request_handler_impl.hpp"
#include "socket.hpp"
#include "tcp_request.hpp"
#include "http_request.hpp"

#include <memory>
#include <list>
#include <mutex>
#include <atomic>

namespace tristan::network {

    class AsyncRequestHandler {
        AsyncRequestHandler();

    public:
        AsyncRequestHandler(const AsyncRequestHandler& other) = delete;
        AsyncRequestHandler(AsyncRequestHandler&& other) = delete;

        AsyncRequestHandler& operator=(const AsyncRequestHandler& other) = delete;
        AsyncRequestHandler& operator=(AsyncRequestHandler&& other) = delete;

        virtual ~AsyncRequestHandler() = default;
        static auto create() -> std::unique_ptr< AsyncRequestHandler >;
        void run();
        void addRequest(std::shared_ptr< NetworkRequestBase >&& network_request);

        void setMaxDownloadsCount(uint8_t count);
        void stop();

    protected:
    private:

        std::mutex m_processed_requests_lock;

        std::list< ResumableCoroutine > m_processed_requests;

        std::unique_ptr<private_::AsyncNetworkRequestHandlerImpl> m_request_handler;

        uint8_t m_max_processed_requests_count;

        std::atomic< bool > m_working;
    };
}  // namespace tristan::network

#endif  // ASYNC_REQUEST_HANDLER_HPP
