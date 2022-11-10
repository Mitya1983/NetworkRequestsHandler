#include "async_tcp_request_handler.hpp"
#include "network_error.hpp"
#include "net_log.hpp"

#include <thread>
#include <limits>
#include <vector>

namespace {

    constexpr uint8_t g_max_frame = std::numeric_limits< uint8_t >::max();

}  // End of anonymous namespace

tristan::network::AsyncTcpRequestHandler::AsyncTcpRequestHandler() :
    m_time_out_interval(std::chrono::seconds(10)),
    m_max_processed_requests_count(50),
    m_working(false) { }

auto tristan::network::AsyncTcpRequestHandler::create() -> std::unique_ptr< AsyncTcpRequestHandler > {
    return std::unique_ptr< AsyncTcpRequestHandler >(new AsyncTcpRequestHandler());
}

void tristan::network::AsyncTcpRequestHandler::setMaxDownloadsCount(uint8_t count) { m_max_processed_requests_count = count; }

void tristan::network::AsyncTcpRequestHandler::setTimeOutInterval(std::chrono::seconds interval) { m_time_out_interval = interval; }

void tristan::network::AsyncTcpRequestHandler::setWorking(bool value) { m_working.store(value, std::memory_order_relaxed); }

void tristan::network::AsyncTcpRequestHandler::run() {
    netInfo("Starting async request handler");
    while (m_working) {
        if (m_processed_requests.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            netDebug("Async request handler queue is empty. Sleeping");
        } else {
            uint8_t active_requests_counter = 0;
            for (auto active_requests_iterator = m_processed_requests.begin(); active_requests_iterator != m_processed_requests.end();) {
                if (active_requests_counter > m_max_processed_requests_count) {
                    continue;
                    netInfo("Maximum async requests number is reached");
                }
                if (not active_requests_iterator->resume()) {
                    std::scoped_lock< std::mutex > lock(m_processed_requests_lock);
                    active_requests_iterator = m_processed_requests.erase(active_requests_iterator);
                    --active_requests_counter;
                } else {
                    ++active_requests_iterator;
                    ++active_requests_counter;
                }
            }
            netDebug("Active async requests number is " + std::to_string(active_requests_counter));
        }
    }
    netInfo("Async request handler stopped");
}

void tristan::network::AsyncTcpRequestHandler::addRequest(std::shared_ptr< NetworkRequest > network_request) {
    if (not m_working) {
        tristan::network::NetworkRequest::ProtectedMembers::pSetError(
            network_request, tristan::network::makeError(tristan::network::ErrorCode::ASYNC_NETWORK_REQUEST_HANDLER_WAS_NOT_LUNCHED));
        return;
    }
    m_processed_requests.emplace_back(AsyncTcpRequestHandler::_processRequest(std::move(network_request)));
}

auto tristan::network::AsyncTcpRequestHandler::_processRequest(std::shared_ptr< tristan::network::NetworkRequest > network_request)
    -> tristan::network::ResumableCoroutine {

    netInfo("Starting processing of request " + network_request->uuid());

    tristan::network::Socket socket;

    if (socket.error()) {
        netError(socket.error().message());
        tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
        co_return;
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::PROCESSED);

    socket.setRemoteIp(network_request->url().hostIP().as_int);
    socket.setRemotePort(network_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    std::chrono::microseconds time_out;
    while (not socket.connected()) {
        if (network_request->isPaused()) {
            netDebug("Network request " + network_request->uuid());
            netInfo("Network request is paused");
            co_return;
        }
        netDebug("Network request " + network_request->uuid());
        netInfo("Connecting to " + network_request->url().hostIP().as_string);
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        socket.connect();

        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::CONNECT_TRY_AGAIN)) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
            co_return;
        }
        auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        time_out += end - start;
        if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                          tristan::network::makeError(tristan::network::SocketErrors::CONNECT_TIMED_OUT));
            co_return;
        }
        co_await std::suspend_always();
    }

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = network_request->requestData().size();

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::WRITING);
    time_out = std::chrono::microseconds(0);
    while (bytes_written < bytes_to_write) {
        if (network_request->isPaused()) {
            netDebug("Network request " + network_request->uuid());
            netInfo("Network request is paused");
            co_return;
        }
        netDebug("Network request " + network_request->uuid());
        netInfo("Writing to " + network_request->url().hostIP().as_string);
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        auto bytes_remain = bytes_to_write - bytes_written;
        uint8_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);

        bytes_written += socket.write(network_request->requestData(), current_frame_size, bytes_written);
        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::WRITE_TRY_AGAIN)) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
            co_return;
        }
        auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        time_out += end - start;
        if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                          tristan::network::makeError(tristan::network::SocketErrors::WRITE_TIMED_OUT));
            co_return;
        }
        netDebug(std::to_string(current_frame_size) + " bytes was written");
        co_await std::suspend_always();
    }

    if (network_request->bytesToRead() != 0) {
        uint64_t bytes_read = 0;
        uint64_t bytes_to_read = network_request->bytesToRead();
        while (bytes_read < bytes_to_read) {
            if (network_request->isPaused()) {
                netDebug("Network request " + network_request->uuid());
                netInfo("Network request is paused");
                co_return;
            }
            auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            auto bytes_remain = bytes_to_read - bytes_read;
            uint8_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);

            auto data = socket.read(current_frame_size);
            if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::READ_TRY_AGAIN)) {
                netError(socket.error().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
                co_return;
            }
            if (not data.empty()) {
                netDebug(std::to_string(data.size()) + " bytes was read");
                netDebug("Data: " + std::string(data.begin(), data.end()));
                bytes_read += data.size();
                tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request, std::move(data));
            }
            auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            time_out += end - start;
            if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
                netError(socket.error().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                              tristan::network::makeError(tristan::network::SocketErrors::READ_TIMED_OUT));
                co_return;
            }
            co_await std::suspend_always();
        }
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::DONE);
    netInfo("Request " + network_request->uuid() + " successfully processed");
}
