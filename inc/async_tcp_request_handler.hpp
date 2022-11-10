#ifndef ASYNC_TCP_REQUEST_HANDLER_HPP
#define ASYNC_TCP_REQUEST_HANDLER_HPP

#include "resumable_coroutine.hpp"
#include "socket.hpp"

#include <memory>
#include <list>
#include <mutex>
#include <atomic>

namespace tristan::network {

    class AsyncTcpRequestHandler {
        AsyncTcpRequestHandler();

    public:
        AsyncTcpRequestHandler(const AsyncTcpRequestHandler& other) = delete;
        AsyncTcpRequestHandler(AsyncTcpRequestHandler&& other) = delete;

        AsyncTcpRequestHandler& operator=(const AsyncTcpRequestHandler& other) = delete;
        AsyncTcpRequestHandler& operator=(AsyncTcpRequestHandler&& other) = delete;

        ~AsyncTcpRequestHandler() = default;
        static auto create() -> std::unique_ptr< AsyncTcpRequestHandler >;
        void run();
        void addRequest(std::shared_ptr< NetworkRequest > network_request);

        void setMaxDownloadsCount(uint8_t count);
        void setTimeOutInterval(std::chrono::seconds interval);
        void setWorking(bool value);

    protected:
        virtual auto _processRequest(std::shared_ptr< tristan::network::NetworkRequest > network_request)
        -> tristan::network::ResumableCoroutine;
    private:

        std::mutex m_processed_requests_lock;

        std::list< ResumableCoroutine > m_processed_requests;

        std::chrono::seconds m_time_out_interval;
        uint8_t m_max_processed_requests_count;

        std::atomic< bool > m_working;
    };
}  // namespace tristan::network

#endif  // ASYNC_TCP_REQUEST_HANDLER_HPP
