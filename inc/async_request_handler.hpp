#ifndef ASYNC_REQUEST_HANDLER_HPP
#define ASYNC_REQUEST_HANDLER_HPP

#include "resumable_coroutine.hpp"
#include "socket.hpp"
#include "network_request.hpp"
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
        void addRequest(std::shared_ptr< NetworkRequest >&& network_request);

        void setMaxDownloadsCount(uint8_t count);
        void setTimeOutInterval(std::chrono::seconds interval);
        void stop();

    protected:
    private:

        auto _processTcpRequest(std::shared_ptr< tristan::network::NetworkRequest > network_request)
        -> tristan::network::ResumableCoroutine;
        auto _processHTTPRequest(std::shared_ptr< tristan::network::HttpRequest > network_request)
        -> tristan::network::ResumableCoroutine;

        std::mutex m_processed_requests_lock;

        std::list< ResumableCoroutine > m_processed_requests;

        std::chrono::seconds m_time_out_interval;
        uint8_t m_max_processed_requests_count;

        std::atomic< bool > m_working;
    };
}  // namespace tristan::network

#endif  // ASYNC_REQUEST_HANDLER_HPP
