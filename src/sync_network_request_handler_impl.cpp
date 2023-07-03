#include "sync_network_request_handler_impl.hpp"
#include "network_logger.hpp"
#include "http_response.hpp"

#include <socket_error.hpp>

#include <thread>

tristan::network::private_::SyncNetworkRequestHandlerImpl::SyncNetworkRequestHandlerImpl() = default;

tristan::network::private_::SyncNetworkRequestHandlerImpl::~SyncNetworkRequestHandlerImpl() = default;
void tristan::network::private_::SyncNetworkRequestHandlerImpl::handleRequest(std::shared_ptr< NetworkRequestBase >&& p_network_request) {
    if (auto tcp_ptr = std::dynamic_pointer_cast< tristan::network::TcpRequest >(p_network_request)) {
        std::thread(&SyncNetworkRequestHandlerImpl::handleTcpRequest, this, std::move(tcp_ptr)).detach();
    } else if (auto http_ptr = std::dynamic_pointer_cast< tristan::network::HttpRequest >(p_network_request)){
        std::thread(&SyncNetworkRequestHandlerImpl::handleHttpRequest, this, std::move(http_ptr)).detach();
    } else {
        this->handleUnimplementedRequest(std::move(p_network_request));
    }
}

// NOLINTNEXTLINE
void tristan::network::private_::SyncNetworkRequestHandlerImpl::handleTcpRequest(std::shared_ptr< TcpRequest >&& p_tcp_request) {
    netTrace("Start");
    netInfo("Starting processing of request " + p_tcp_request->uuid());

    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(p_tcp_request);

    tristan::sockets::InetSocket socket;

    if (socket.error()) {
        netError(socket.error().message());
        p_tcp_request->request_handlers_api.setError(socket.error());
        return;
    }

    p_tcp_request->request_handlers_api.setStatus(tristan::network::Status::PROCESSED);

    socket.setHost(p_tcp_request->url().hostIP().as_int, p_tcp_request->url().host());
    socket.setPort(p_tcp_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (not socket.connected()) {
        if (p_tcp_request->isPaused()) {
            netInfo("Network request is paused tcp_request->uuid() = " + p_tcp_request->uuid());
            return;
        }
        if (p_tcp_request->isCanceled()) {
            netInfo("Network request is cancelled tcp_request->uuid() = " + p_tcp_request->uuid());
            return;
        }
        netInfo("Connecting to " + p_tcp_request->url().hostIP().as_string);
        netDebug("tcp_request->uuid() = " + p_tcp_request->uuid());

        socket.connect();

        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_tcp_request)) {
            return;
        }
        if (not socket.connected()) {
            socket.resetError();
            netDebug("Sleeping on connect");
            std::this_thread::sleep_for(std::chrono::milliseconds(m_sleeping_interval));
        }
    }

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = p_tcp_request->requestData().size();

    p_tcp_request->request_handlers_api.setStatus(tristan::network::Status::WRITING);
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (bytes_written < bytes_to_write) {
        if (p_tcp_request->isPaused()) {
            netInfo("Network request is paused tcp_request->uuid() = " + p_tcp_request->uuid());
            return;
        }
        if (p_tcp_request->isCanceled()) {
            netInfo("Network request is cancelled tcp_request->uuid() = " + p_tcp_request->uuid());
            return;
        }
        netInfo("Writing to " + p_tcp_request->url().hostIP().as_string);
        auto bytes_remain = bytes_to_write - bytes_written;
        uint16_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
        bytes_written += socket.write(p_tcp_request->requestData(), current_frame_size, bytes_written);
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_tcp_request)) {
            return;
        }
        if (socket.error()) {
            socket.resetError();
            netDebug("Sleeping on write");
            std::this_thread::sleep_for(std::chrono::milliseconds(m_sleeping_interval));
        }
    }

    if (p_tcp_request->bytesToRead() != 0) {
        uint64_t bytes_read = 0;
        uint64_t bytes_to_read = p_tcp_request->bytesToRead();
        start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        while (bytes_read < bytes_to_read) {
            if (p_tcp_request->isPaused()) {
                netInfo("Network request is paused tcp_request->uuid() = " + p_tcp_request->uuid());
                return;
            }
            if (p_tcp_request->isCanceled()) {
                netInfo("Network request is cancelled tcp_request->uuid() = " + p_tcp_request->uuid());
                return;
            }
            auto bytes_remain = bytes_to_read - bytes_read;
            uint16_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
            auto data = socket.read(current_frame_size);

            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_tcp_request)) {
                return;
            }
            if (not data.empty()) {
                netDebug("data.size() = " + std::to_string(data.size()));
                netDebug("data = " + std::string(data.begin(), data.end()));
                bytes_read += data.size();
                p_tcp_request->request_handlers_api.addResponseData(std::move(data));
                if (p_tcp_request->error()) {
                    netError(p_tcp_request->error().message());
                    return;
                }
            }
            if (socket.error()) {
                socket.resetError();
                netDebug("Sleeping on read");
                std::this_thread::sleep_for(std::chrono::milliseconds(m_sleeping_interval));
            }
        }
    }

    p_tcp_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
    netInfo("Request " + p_tcp_request->uuid() + " successfully processed");

    netTrace("End");
}

// NOLINTNEXTLINE
void tristan::network::private_::SyncNetworkRequestHandlerImpl::handleHttpRequest(std::shared_ptr< HttpRequest >&& p_http_request) {
    netTrace("Start");

    netInfo("Starting processing of HTTP request " + p_http_request->uuid());

    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(p_http_request);

    tristan::sockets::InetSocket socket;

    if (socket.error()) {
        netError(socket.error().message());
        p_http_request->request_handlers_api.setError(socket.error());
        return;
    }

    p_http_request->request_handlers_api.setStatus(tristan::network::Status::PROCESSED);

    socket.setHost(p_http_request->url().hostIP().as_int, p_http_request->url().host());
    socket.setPort(p_http_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (not socket.connected()) {
        if (p_http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
            return;
        }
        if (p_http_request->isCanceled()) {
            netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
            return;
        }
        netInfo("Connecting to " + p_http_request->url().hostIP().as_string);
        socket.connect(p_http_request->isSSL());

        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
            return;
        }
        if (socket.error()) {
            socket.resetError();
            netDebug("Sleeping on connect");
            std::this_thread::sleep_for(m_sleeping_interval);
        }
    }

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = p_http_request->requestData().size();
    p_http_request->request_handlers_api.setStatus(tristan::network::Status::WRITING);
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (bytes_written < bytes_to_write) {
        if (p_http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
            return;
        }
        if (p_http_request->isCanceled()) {
            netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
            return;
        }
        netInfo("Writing to " + p_http_request->url().hostIP().as_string);
        auto bytes_remain = bytes_to_write - bytes_written;
        uint16_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
        bytes_written += socket.write(p_http_request->requestData(), current_frame_size, bytes_written);
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
            return;
        }
        if (socket.error()) {
            socket.resetError();
            netDebug("Sleeping on write");
            std::this_thread::sleep_for(m_sleeping_interval);
        }
        netDebug(std::to_string(current_frame_size) + " bytes was written");
    }

    p_http_request->request_handlers_api.setStatus(tristan::network::Status::READING);
    std::vector<uint8_t> headers_data;
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (true) {
        if (p_http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
            return;
        }
        if (p_http_request->isCanceled()) {
            netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
            return;
        }
        auto data = socket.readUntil({'\r', '\n', '\r', '\n'});
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
            return;
        }
        if (socket.error() && socket.error().value() != static_cast< int >(tristan::sockets::Error::READ_DONE)) {
            if (not data.empty()) {
                netDebug(std::to_string(data.size()) + " bytes was read");
                netDebug("Data: " + std::string(data.begin(), data.end()));
                headers_data.insert(headers_data.end(), data.begin(), data.end());
            }
            socket.resetError();
            netDebug("Sleeping on read until");
            std::this_thread::sleep_for(m_sleeping_interval);
            continue;
        }
        if (not data.empty()) {
            netDebug(std::to_string(data.size()) + " bytes was read");
            netDebug("Data: " + std::string(data.begin(), data.end()));
            headers_data.insert(headers_data.end(), data.begin(), data.end());
        }
        p_http_request->initResponse(std::move(headers_data));
        if (p_http_request->error()) {
            netError(p_http_request->error().message());
            return;
        }
        break;
    }

    auto response = std::dynamic_pointer_cast< tristan::network::HttpResponse >(p_http_request->response());
    if (not response) {
        netFatal("Bad dynamic cast");
        throw std::bad_cast();
    }
    if (response->status() != tristan::network::HttpStatus::Ok) {
        p_http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
        return;
    }

    if (auto content_length = response->headers()->headerValue(tristan::network::http::header_names::content_length)) {
        p_http_request->setBytesToRead(std::stoll(content_length.value()));
        netInfo("Content-length header found");
        if (p_http_request->bytesToRead() != 0) {
            socket.resetError();
            p_http_request->request_handlers_api.setStatus(tristan::network::Status::READING);
            uint64_t bytes_read = 0;
            uint64_t bytes_to_read = p_http_request->bytesToRead();
            start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            while (bytes_read < bytes_to_read) {
                if (p_http_request->isPaused()) {
                    netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
                    return;
                }
                if (p_http_request->isCanceled()) {
                    netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
                    return;
                }
                auto bytes_remain = bytes_to_read - bytes_read;
                //ERROR: uint8_t should be changed to uint16_t according to m_max_frame_size
                uint16_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
                auto data = socket.read(current_frame_size);
                if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
                    return;
                }
                if (not data.empty()) {
                    netDebug(std::to_string(data.size()) + " bytes was read from " + p_http_request->url().hostIP().as_string);
                    bytes_read += data.size();
                    p_http_request->request_handlers_api.addResponseData(std::move(data));
                    if (p_http_request->error()) {
                        netError(p_http_request->error().message());
                        return;
                    }
                }
                if (socket.error()) {
                    socket.resetError();
                    netDebug("Sleeping on read");
                    std::this_thread::sleep_for(std::chrono::milliseconds(m_sleeping_interval));
                }
            }
        } else {
            netWarning("Content length header contained 0 value");
            p_http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
        }
    } else if (auto transfer_encoding = response->headers()->headerValue(tristan::network::http::header_names::transfer_encoding)) {
        netInfo("Transfer-encoding header found");
        if (transfer_encoding.value().find("chunked") == std::string::npos) {
            netWarning("Transfer-encoding header does not contain chunked specification.");
            p_http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
            return;
        }
        while (true) {
            if (p_http_request->isPaused()) {
                netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
                return;
            }
            if (p_http_request->isCanceled()) {
                netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
                return;
            }
            auto chunk_size = socket.readUntil({'\r', '\n'});
            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
                return;
            }
            if (socket.error() && socket.error().value() != static_cast< int >(tristan::sockets::Error::READ_DONE)) {
                socket.resetError();
                netDebug("Sleeping on read until");
                std::this_thread::sleep_for(m_sleeping_interval);
                continue;
            }
            auto pos = std::find(chunk_size.begin(), chunk_size.end(), ';');
            uint64_t bytes_to_read = std::stoll(std::string(chunk_size.begin(), pos), nullptr, 16);
            if (bytes_to_read == 0) {
                break;
            }
            socket.resetError();
            uint64_t bytes_read = 0;
            start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            while (bytes_read < bytes_to_read) {
                if (p_http_request->isPaused()) {
                    netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
                    return;
                }
                if (p_http_request->isCanceled()) {
                    netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
                    return;
                }
                auto bytes_remain = bytes_to_read - bytes_read;
                uint16_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
                auto data = socket.read(current_frame_size);
                if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
                    return;
                }
                if (not data.empty()) {
                    netDebug(std::to_string(data.size()) + " bytes was read from " + p_http_request->url().hostIP().as_string);
                    bytes_read += data.size();
                    p_http_request->request_handlers_api.addResponseData(std::move(data));
                    if (p_http_request->error()) {
                        netError(p_http_request->error().message());
                        return;
                    }
                }
                if (socket.error()) {
                    socket.resetError();
                    netDebug("Sleeping on read");
                    std::this_thread::sleep_for(std::chrono::milliseconds(m_sleeping_interval));
                }
            }
            auto redundant_data = socket.read(2);
            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
                return;
            }
            if (socket.error()) {
                socket.resetError();
                netDebug("Sleeping on read");
                std::this_thread::sleep_for(std::chrono::milliseconds(m_sleeping_interval));
            }
        }
    } else {
        p_http_request->request_handlers_api.setError(tristan::network::makeError(tristan::network::NetworkResponseError::HTTP_RESPONSE_SIZE_ERROR));
        return;
    }
    p_http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
    netInfo("Request " + p_http_request->uuid() + " successfully processed");
    netTrace("End");
}

void tristan::network::private_::SyncNetworkRequestHandlerImpl::handleUnimplementedRequest(
    std::shared_ptr< tristan::network::NetworkRequestBase >&& p_network_request) {
    netError("Unimplemented network request received");
    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(p_network_request);
    p_network_request->request_handlers_api.setStatus(tristan::network::Status::ERROR);
    p_network_request->request_handlers_api.setError(tristan::network::makeError(tristan::network::ErrorCode::REQUEST_NOT_SUPPORTED));
}
