#include "tcp_request_handler.hpp"
#include "net_log.hpp"

namespace {
    constexpr uint8_t g_max_frame = std::numeric_limits< uint8_t >::max();
}  //End of anonymous namespace

tristan::network::TcpRequestHandler::TcpRequestHandler(std::chrono::seconds time_out_interval) :
    m_time_out_interval(time_out_interval) { }

void tristan::network::TcpRequestHandler::processRequest(std::shared_ptr< tristan::network::NetworkRequest >&& network_request) {

    netInfo("Starting processing of request " + network_request->uuid());
    netDebug("network_request->uuid() = " + network_request->uuid());
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
        return;
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::PROCESSED);

    socket.setHost(network_request->url().hostIP().as_int, network_request->url().host());
    socket.setRemotePort(network_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    std::chrono::microseconds time_out;
    while (not socket.connected()) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            return;
        }
        netInfo("Connecting to " + network_request->url().hostIP().as_string);
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        socket.connect();

        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::CONNECT_TRY_AGAIN)
            && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::CONNECT_IN_PROGRESS)) {
            netError(socket.error().message());
            netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
            return;
        }
        auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        time_out += end - start;
        if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                          tristan::network::makeError(tristan::network::SocketErrors::CONNECT_TIMED_OUT));
            return;
        }
    }
    socket.resetError();

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = network_request->requestData().size();

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::WRITING);

    time_out = std::chrono::microseconds(0);
    while (bytes_written < bytes_to_write) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            return;
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
            return;
        } else if (socket.error().value() == static_cast< int >(tristan::network::SocketErrors::WRITE_TRY_AGAIN)) {
            auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            time_out += end - start;
            if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
                netError(socket.error().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                              tristan::network::makeError(tristan::network::SocketErrors::WRITE_TIMED_OUT));
                return;
            }
        }
        netDebug(std::to_string(current_frame_size) + " bytes was written");
        socket.resetError();
    }

    if (network_request->bytesToRead() != 0) {
        socket.resetError();
        tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::READING);
        uint64_t bytes_read = 0;
        uint64_t bytes_to_read = network_request->bytesToRead();
        while (bytes_read < bytes_to_read) {
            if (network_request->isPaused()) {
                netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
                return;
            }
            auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            auto bytes_remain = bytes_to_read - bytes_read;
            uint8_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);
            auto data = socket.read(current_frame_size);
            if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::READ_TRY_AGAIN)) {
                netError(socket.error().message());
                netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
                return;
            } else if (socket.error().value() == static_cast< int >(tristan::network::SocketErrors::READ_TRY_AGAIN)){
                auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
                time_out += end - start;
                if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
                    netError(socket.error().message());
                    tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                                  tristan::network::makeError(tristan::network::SocketErrors::READ_TIMED_OUT));
                    return;
                }
            }
            if (not data.empty()) {
                netDebug(std::to_string(data.size()) + " bytes was read");
                netDebug("Data: " + std::string(data.begin(), data.end()));
                bytes_read += data.size();
                tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request, std::move(data));
            }
            socket.resetError();
        }
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::DONE);
    netInfo("Request " + network_request->uuid() + " successfully processed");
}
