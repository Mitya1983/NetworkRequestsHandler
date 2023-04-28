#ifndef ASYNC_REQUEST_HANDLER_HPP
#define ASYNC_REQUEST_HANDLER_HPP

#include "tcp_request.hpp"
#include "http_request.hpp"

#include <resumable_coroutine.hpp>

#include <memory>
#include <list>
#include <mutex>
#include <atomic>

namespace tristan::network::private_ {

//    namespace private_ {
        class AsyncNetworkRequestHandlerImpl;
//    } //End of private_ namespace

    class AsyncRequestHandler {
        AsyncRequestHandler();

    public:
        AsyncRequestHandler(const AsyncRequestHandler& other) = delete;
        AsyncRequestHandler(AsyncRequestHandler&& other) = delete;

        AsyncRequestHandler& operator=(const AsyncRequestHandler& other) = delete;
        AsyncRequestHandler& operator=(AsyncRequestHandler&& other) = delete;

        virtual ~AsyncRequestHandler();
        static auto create() -> std::unique_ptr< AsyncRequestHandler >;
        void run();
        void addRequest(std::shared_ptr< NetworkRequestBase >&& network_request);

        void setMaxDownloadsCount(uint8_t count);
        void stop();

    protected:
    private:

        std::mutex m_processed_requests_lock;

        std::list< tristan::ResumableCoroutine > m_processed_requests;

        std::unique_ptr<private_::AsyncNetworkRequestHandlerImpl> m_request_handler;

        uint8_t m_max_processed_requests_count;

        std::atomic< bool > m_working;
    };
}  // namespace tristan::network

#endif  // ASYNC_REQUEST_HANDLER_HPP
