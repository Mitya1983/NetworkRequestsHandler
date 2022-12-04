#include "async_request_handler.hpp"
#include "network_error.hpp"
#include "net_log.hpp"

#include <thread>
#include <limits>
#include <vector>

namespace {

    constexpr uint8_t g_max_frame = std::numeric_limits< uint8_t >::max();

}  // End of anonymous namespace

tristan::network::AsyncRequestHandler::AsyncRequestHandler() :
    m_time_out_interval(std::chrono::seconds(10)),
    m_max_processed_requests_count(50),
    m_working(false) { }

auto tristan::network::AsyncRequestHandler::create() -> std::unique_ptr< AsyncRequestHandler > {
    return std::unique_ptr< AsyncRequestHandler >(new AsyncRequestHandler());
}

void tristan::network::AsyncRequestHandler::setMaxDownloadsCount(uint8_t count) { m_max_processed_requests_count = count; }

void tristan::network::AsyncRequestHandler::setTimeOutInterval(std::chrono::seconds interval) { m_time_out_interval = interval; }

void tristan::network::AsyncRequestHandler::stop() { m_working.store(false, std::memory_order_relaxed); }

void tristan::network::AsyncRequestHandler::run() {
    netInfo("Starting async request handler");
    m_working.store(true, std::memory_order_relaxed);

    while (m_working) {
        if (m_processed_requests.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
        }
    }
    netInfo("Async request handler stopped");
}

void tristan::network::AsyncRequestHandler::addRequest(std::shared_ptr< NetworkRequest >&& network_request) {
    if (not m_working) {
        tristan::network::NetworkRequest::ProtectedMembers::pSetError(
            network_request, tristan::network::makeError(tristan::network::ErrorCode::ASYNC_NETWORK_REQUEST_HANDLER_WAS_NOT_LUNCHED));
        return;
    }
    std::scoped_lock< std::mutex > lock(m_processed_requests_lock);
    m_processed_requests.emplace_back(AsyncRequestHandler::_processTcpRequest(std::move(network_request)));
}

auto tristan::network::AsyncRequestHandler::_processTcpRequest(std::shared_ptr< tristan::network::NetworkRequest > network_request)
    -> tristan::network::ResumableCoroutine {

    netInfo("Starting processing of TCP request " + network_request->uuid());
    netDebug("network_request->uuid() = " + network_request->uuid());
    netDebug("network_request->url().host() = " + network_request->url().host());
    netDebug("network_request->url().hostIP().as_string = " + network_request->url().hostIP().as_string);
    netDebug("network_request->url().port() = " + network_request->url().port());
    netDebug("network_request->url() = " + network_request->url().composeUrl());
    netDebug("network_request->requestData() = " + std::string(network_request->requestData().begin(), network_request->requestData().end()));
    netDebug("network_request->requestData().size() = " + std::to_string(network_request->requestData().size()));
    netDebug("network_request->bytesToRead() = " + std::to_string(network_request->bytesToRead()));
    netDebug("network_request->responseDelimiter() = " + std::string(network_request->responseDelimiter().begin(), network_request->responseDelimiter().end()));
    tristan::network::Socket socket;

    if (socket.error()) {
        netError(socket.error().message());
        tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
        co_return;
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::PROCESSED);

    socket.setHost(network_request->url().hostIP().as_int, network_request->url().host());
    socket.setRemotePort(network_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    std::chrono::microseconds time_out;
    while (not socket.connected()) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            co_return;
        }
        netInfo("Connecting to " + network_request->url().hostIP().as_string);
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        socket.connect(network_request->isSSL());

        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::CONNECT_TRY_AGAIN)
            && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::CONNECT_IN_PROGRESS)) {
            netError(socket.error().message());
            netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
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
    co_await std::suspend_always();
    socket.resetError();

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = network_request->requestData().size();

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::WRITING);
    time_out = std::chrono::microseconds(0);
    while (bytes_written < bytes_to_write) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            co_return;
        }
        netInfo("Writing to " + network_request->url().hostIP().as_string);
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        auto bytes_remain = bytes_to_write - bytes_written;
        uint8_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);

        bytes_written += socket.write(network_request->requestData(), current_frame_size, bytes_written);
        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::WRITE_TRY_AGAIN)) {
            netError(socket.error().message());
            netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
            co_return;
        } else if (socket.error().value() == static_cast< int >(tristan::network::SocketErrors::WRITE_TRY_AGAIN)) {
            auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            time_out += end - start;
            if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
                netError(socket.error().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                              tristan::network::makeError(tristan::network::SocketErrors::WRITE_TIMED_OUT));
                co_return;
            }
        }
        netDebug(std::to_string(current_frame_size) + " bytes was written");
        socket.resetError();
        co_await std::suspend_always();
    }

    if (network_request->bytesToRead() != 0) {
        co_await std::suspend_always();
        socket.resetError();
        tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::READING);
        uint64_t bytes_read = 0;
        uint64_t bytes_to_read = network_request->bytesToRead();
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        while (bytes_read < bytes_to_read) {
            if (network_request->isPaused()) {
                netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
                co_return;
            }
            auto bytes_remain = bytes_to_read - bytes_read;
            uint8_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);

            auto data = socket.read(current_frame_size);
            if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::READ_TRY_AGAIN)) {
                netError(socket.error().message());
                netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
                co_return;
            } else if (socket.error().value() == static_cast< int >(tristan::network::SocketErrors::READ_TRY_AGAIN)) {
                auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
                time_out += end - start;
                if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
                    netError(socket.error().message());
                    tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                                  tristan::network::makeError(tristan::network::SocketErrors::READ_TIMED_OUT));
                    co_return;
                }
            }
            if (not data.empty()) {
                netDebug(std::to_string(data.size()) + " bytes was read");
                netDebug("Data: " + std::string(data.begin(), data.end()));
                bytes_read += data.size();
                tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request, std::move(data));
                if (network_request->error()){
                    netError(network_request->error().message());
                    co_return;
                }
            }
            socket.resetError();
            co_await std::suspend_always();
        }
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::DONE);
    netInfo("Request " + network_request->uuid() + " successfully processed");
}

auto tristan::network::AsyncRequestHandler::_processHTTPRequest(std::shared_ptr< tristan::network::HttpRequest > network_request)
    -> tristan::network::ResumableCoroutine {
    netInfo("Starting processing of HTTP request " + network_request->uuid());
    netDebug("network_request->uuid() = " + network_request->uuid());
    netDebug("network_request->url().host() = " + network_request->url().host());
    netDebug("network_request->url().hostIP().as_string = " + network_request->url().hostIP().as_string);
    netDebug("network_request->url().port() = " + network_request->url().port());
    netDebug("network_request->url() = " + network_request->url().composeUrl());
    netDebug("network_request->requestData() = " + std::string(network_request->requestData().begin(), network_request->requestData().end()));
    netDebug("network_request->requestData().size() = " + std::to_string(network_request->requestData().size()));
    netDebug("network_request->bytesToRead() = " + std::to_string(network_request->bytesToRead()));
    netDebug("network_request->responseDelimiter() = " + std::string(network_request->responseDelimiter().begin(), network_request->responseDelimiter().end()));

    tristan::network::Socket socket;

    if (socket.error()) {
        netError(socket.error().message());
        tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
        co_return;
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::PROCESSED);

    socket.setHost(network_request->url().hostIP().as_int, network_request->url().host());
    socket.setRemotePort(network_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    std::chrono::microseconds time_out;
    while (not socket.connected()) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            co_return;
        }
        netInfo("Connecting to " + network_request->url().hostIP().as_string);
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        socket.connect(network_request->isSSL());

        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::CONNECT_TRY_AGAIN)
            && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::CONNECT_IN_PROGRESS)) {
            netError(socket.error().message());
            netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
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
    co_await std::suspend_always();
    socket.resetError();

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = network_request->requestData().size();

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::WRITING);
    time_out = std::chrono::microseconds(0);
    while (bytes_written < bytes_to_write) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            co_return;
        }
        netInfo("Writing to " + network_request->url().hostIP().as_string);
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        auto bytes_remain = bytes_to_write - bytes_written;
        uint8_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);

        bytes_written += socket.write(network_request->requestData(), current_frame_size, bytes_written);
        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::WRITE_TRY_AGAIN)) {
            netError(socket.error().message());
            netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
            co_return;
        } else if (socket.error().value() == static_cast< int >(tristan::network::SocketErrors::WRITE_TRY_AGAIN)) {
            auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            time_out += end - start;
            if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
                netError(socket.error().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                              tristan::network::makeError(tristan::network::SocketErrors::WRITE_TIMED_OUT));
                co_return;
            }
        }
        netDebug(std::to_string(current_frame_size) + " bytes was written");
        socket.resetError();
        co_await std::suspend_always();
    }

    co_await std::suspend_always();
    socket.resetError();
    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::READING);

    auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (true) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            co_return;
        }
        auto data = socket.readUntil({'\r', '\n', '\r', '\n'});
        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::READ_TRY_AGAIN)) {
            netError(socket.error().message());
            netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
            co_return;
        } else if (socket.error().value() == static_cast< int >(tristan::network::SocketErrors::READ_TRY_AGAIN)) {
            auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            time_out += end - start;
            if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
                netError(socket.error().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                              tristan::network::makeError(tristan::network::SocketErrors::READ_TIMED_OUT));
                co_return;
            }
        }
        if (not data.empty()){
            netDebug(std::to_string(data.size()) + " bytes was read");
            netDebug("Data: " + std::string(data.begin(), data.end()));
            tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request, std::move(data));
            if (network_request->error()){
                netError(network_request->error().message());
                co_return;
            }
        }
        socket.resetError();
        co_await std::suspend_always();
    }



    //TODO: read by size if header contains content-length header
    //TODO: read by chunks if header contains content-length chunked
}
