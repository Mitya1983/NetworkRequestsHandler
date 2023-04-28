#include "async_network_request_handler_impl.hpp"
#include "network_logger.hpp"
#include "http_response.hpp"

tristan::network::private_::AsyncNetworkRequestHandlerImpl::AsyncNetworkRequestHandlerImpl() = default;

tristan::network::private_::AsyncNetworkRequestHandlerImpl::~AsyncNetworkRequestHandlerImpl() = default;

auto tristan::network::private_::AsyncNetworkRequestHandlerImpl::handleRequest(std::shared_ptr< NetworkRequestBase >&& network_request)
    -> tristan::ResumableCoroutine {
    if (auto tcp_ptr = std::dynamic_pointer_cast< tristan::network::TcpRequest >(network_request)) {
        return handleTcpRequest(tcp_ptr);
    } else if (auto http_ptr = std::dynamic_pointer_cast< tristan::network::HttpRequest >(network_request)) {
        return handleHTTPRequest(http_ptr);
    }
    return handleUnimplementedRequest(network_request);
}

auto tristan::network::private_::AsyncNetworkRequestHandlerImpl::handleTcpRequest(std::shared_ptr< tristan::network::TcpRequest > tcp_request)
    -> tristan::ResumableCoroutine {
    netInfo("Starting processing of request " + tcp_request->uuid());

    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(tcp_request);

    tristan::sockets::InetSocket socket;

    if (socket.error()) {
        netError(socket.error().message());
        tcp_request->request_handlers_api.setError(socket.error());
        co_return;
    }

    tcp_request->request_handlers_api.setStatus(tristan::network::Status::PROCESSED);

    socket.setHost(tcp_request->url().hostIP().as_int, tcp_request->url().host());
    socket.setPort(tcp_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (not socket.connected()) {
        if (tcp_request->isPaused()) {
            netInfo("Network request is paused tcp_request->uuid() = " + tcp_request->uuid());
            co_return;
        }
        netInfo("Connecting to " + tcp_request->url().hostIP().as_string);
        netDebug("tcp_request->uuid() = " + tcp_request->uuid());

        socket.connect();

        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, tcp_request)) {
            co_return;
        }
        co_await std::suspend_always();
        socket.resetError();
    }

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = tcp_request->requestData().size();

    tcp_request->request_handlers_api.setStatus(tristan::network::Status::WRITING);
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (bytes_written < bytes_to_write) {
        if (tcp_request->isPaused()) {
            netInfo("Network request is paused tcp_request->uuid() = " + tcp_request->uuid());
            co_return;
        }
        netInfo("Writing to " + tcp_request->url().hostIP().as_string);
        auto bytes_remain = bytes_to_write - bytes_written;
        uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
        bytes_written += socket.write(tcp_request->requestData(), current_frame_size, bytes_written);
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, tcp_request)) {
            co_return;
        }
        co_await std::suspend_always();
        socket.resetError();
    }

    if (tcp_request->bytesToRead() != 0) {
        uint64_t bytes_read = 0;
        uint64_t bytes_to_read = tcp_request->bytesToRead();
        start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        while (bytes_read < bytes_to_read) {
            if (tcp_request->isPaused()) {
                netInfo("Network request is paused tcp_request->uuid() = " + tcp_request->uuid());
                co_return;
            }
            auto bytes_remain = bytes_to_read - bytes_read;
            uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
            auto data = socket.read(current_frame_size);

            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, tcp_request)) {
                co_return;
            }
            if (not data.empty()) {
                netDebug("data.size() = " + std::to_string(data.size()));
                netDebug("data = " + std::string(data.begin(), data.end()));
                bytes_read += data.size();
                tcp_request->request_handlers_api.addResponseData(std::move(data));
                if (tcp_request->error()) {
                    netError(tcp_request->error().message());
                    co_return;
                }
            }
            co_await std::suspend_always();
            socket.resetError();
        }
    }
    tcp_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
    netInfo("Request " + tcp_request->uuid() + " successfully processed");
}

auto tristan::network::private_::AsyncNetworkRequestHandlerImpl::handleHTTPRequest(std::shared_ptr< tristan::network::HttpRequest > http_request)
    -> tristan::ResumableCoroutine {
    netInfo("Starting processing of HTTP request " + http_request->uuid());

    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(http_request);

    tristan::sockets::InetSocket socket;

    if (socket.error()) {
        netError(socket.error().message());
        http_request->request_handlers_api.setError(socket.error());
        co_return;
    }

    http_request->request_handlers_api.setStatus(tristan::network::Status::PROCESSED);

    socket.setHost(http_request->url().hostIP().as_int, http_request->url().host());
    socket.setPort(http_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (not socket.connected()) {
        if (http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + http_request->uuid());
            co_return;
        }
        netInfo("Connecting to " + http_request->url().hostIP().as_string);
        socket.connect(http_request->isSSL());

        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, http_request)) {
            co_return;
        }
        co_await std::suspend_always();
        socket.resetError();
    }

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = http_request->requestData().size();
    http_request->request_handlers_api.setStatus(tristan::network::Status::WRITING);
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (bytes_written < bytes_to_write) {
        if (http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + http_request->uuid());
            co_return;
        }
        netInfo("Writing to " + http_request->url().hostIP().as_string);
        auto bytes_remain = bytes_to_write - bytes_written;
        uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
        bytes_written += socket.write(http_request->requestData(), current_frame_size, bytes_written);
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, http_request)) {
            co_return;
        }
        netDebug(std::to_string(current_frame_size) + " bytes was written");
        co_await std::suspend_always();
        socket.resetError();
    }

    http_request->request_handlers_api.setStatus(tristan::network::Status::READING);

    std::vector< uint8_t > headers_data;
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (true) {
        if (http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + http_request->uuid());
            co_return;
        }
        auto data = socket.readUntil({'\r', '\n', '\r', '\n'});
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, http_request)) {
            co_return;
        }
        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::READ_DONE)) {
            if (not data.empty()) {
                netDebug(std::to_string(data.size()) + " bytes was read");
                netDebug("Data: " + std::string(data.begin(), data.end()));
                headers_data.insert(headers_data.end(), data.begin(), data.end());
            }
            co_await std::suspend_always();
            socket.resetError();
            continue;
        }
        if (not data.empty()) {
            netDebug(std::to_string(data.size()) + " bytes was read");
            netDebug("Data: " + std::string(data.begin(), data.end()));
            headers_data.insert(headers_data.end(), data.begin(), data.end());
        }
        http_request->initResponse(std::move(headers_data));
        if (http_request->error()) {
            netError(http_request->error().message());
            co_return;
        }
        break;
    }

    auto response = std::dynamic_pointer_cast< tristan::network::HttpResponse >(http_request->response());
    if (not response) {
        netFatal("Bad dynamic cast");
        std::exit(1);
    }
    if (response->status() != tristan::network::HttpStatus::Ok) {
        http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
        co_return;
    }

    if (auto content_length = response->headers()->headerValue(tristan::network::http::header_names::content_length)) {
        http_request->setBytesToRead(std::stoll(content_length.value()));
        netInfo("Content-length header found");
        if (http_request->bytesToRead() != 0) {
            socket.resetError();
            http_request->request_handlers_api.setStatus(tristan::network::Status::READING);
            uint64_t bytes_read = 0;
            uint64_t bytes_to_read = http_request->bytesToRead();
            start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            while (bytes_read < bytes_to_read) {
                if (http_request->isPaused()) {
                    netInfo("Network request is paused http_request->uuid() = " + http_request->uuid());
                    co_return;
                }
                auto bytes_remain = bytes_to_read - bytes_read;
                uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);

                auto data = socket.read(current_frame_size);
                if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, http_request)) {
                    co_return;
                }
                if (not data.empty()) {
                    netDebug(std::to_string(data.size()) + " bytes was read from " + http_request->url().hostIP().as_string);
                    bytes_read += data.size();
                    http_request->request_handlers_api.addResponseData(std::move(data));
                    if (http_request->error()) {
                        netError(http_request->error().message());
                        co_return;
                    }
                }
                if (socket.error()) {
                    co_await std::suspend_always();
                    socket.resetError();
                }
            }
        } else {
            netWarning("Content length header contained 0 value");
            http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
        }
    } else if (auto transfer_encoding = response->headers()->headerValue(tristan::network::http::header_names::transfer_encoding)) {
        netInfo("Transfer-encoding header found");
        if (transfer_encoding.value().find("chunked") == std::string::npos) {
            netWarning("Transfer-encoding header does not contain chunked specification.");
            http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
            co_return;
        }
        while (true) {
            if (http_request->isPaused()) {
                netInfo("Network request is paused http_request->uuid() = " + http_request->uuid());
                co_return;
            }
            auto chunk_size = socket.readUntil({'\r', '\n'});
            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, http_request)) {
                co_return;
            }
            if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::READ_DONE)) {
                co_await std::suspend_always();
                socket.resetError();
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
                if (http_request->isPaused()) {
                    netInfo("Network request is paused http_request->uuid() = " + http_request->uuid());
                    co_return;
                }
                auto bytes_remain = bytes_to_read - bytes_read;
                uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
                auto data = socket.read(current_frame_size);
                if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, http_request)) {
                    co_return;
                }
                if (not data.empty()) {
                    netDebug(std::to_string(data.size()) + " bytes was read from " + http_request->url().hostIP().as_string);
                    bytes_read += data.size();
                    http_request->request_handlers_api.addResponseData(std::move(data));
                    if (http_request->error()) {
                        netError(http_request->error().message());
                        co_return;
                    }
                }
                if (socket.error()) {
                    co_await std::suspend_always();
                    socket.resetError();
                }
            }
            auto redundant_data = socket.read(2);
            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, http_request)) {
                co_return;
            }
            if (socket.error()) {
                co_await std::suspend_always();
                socket.resetError();
            }
        }
    } else {
        http_request->request_handlers_api.setError(tristan::network::makeError(tristan::network::NetworkResponseError::HTTP_RESPONSE_SIZE_ERROR));
        co_return;
    }
    http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
    netInfo("Request " + http_request->uuid() + " successfully processed");
}

auto tristan::network::private_::AsyncNetworkRequestHandlerImpl::handleUnimplementedRequest(//NOLINT
    std::shared_ptr< tristan::network::NetworkRequestBase > network_request) -> tristan::ResumableCoroutine {
    netError("Unimplemented network request received");
    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(network_request);
    network_request->request_handlers_api.setStatus(tristan::network::Status::ERROR);
    network_request->request_handlers_api.setError(tristan::network::makeError(tristan::network::ErrorCode::REQUEST_NOT_SUPPORTED));
    co_return;
}
