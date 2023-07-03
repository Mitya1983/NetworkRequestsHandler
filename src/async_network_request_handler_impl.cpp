#include "async_network_request_handler_impl.hpp"
#include "network_logger.hpp"
#include "http_response.hpp"

#include <socket_error.hpp>

tristan::network::private_::AsyncNetworkRequestHandlerImpl::AsyncNetworkRequestHandlerImpl() = default;

tristan::network::private_::AsyncNetworkRequestHandlerImpl::~AsyncNetworkRequestHandlerImpl() = default;

auto tristan::network::private_::AsyncNetworkRequestHandlerImpl::handleRequest(std::shared_ptr< NetworkRequestBase >&& p_network_request)
    -> tristan::ResumableCoroutine {
    if (auto tcp_ptr = std::dynamic_pointer_cast< tristan::network::TcpRequest >(p_network_request)) {
        return handleTcpRequest(tcp_ptr);
    } else if (auto http_ptr = std::dynamic_pointer_cast< tristan::network::HttpRequest >(p_network_request)) {
        return handleHTTPRequest(http_ptr);
    }
    return handleUnimplementedRequest(p_network_request);
}

auto tristan::network::private_::AsyncNetworkRequestHandlerImpl::handleTcpRequest(std::shared_ptr< tristan::network::TcpRequest > p_tcp_request)
    -> tristan::ResumableCoroutine {
    netInfo("Starting processing of request " + p_tcp_request->uuid());

    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(p_tcp_request);

    tristan::sockets::InetSocket socket;

    if (socket.error()) {
        netError(socket.error().message());
        p_tcp_request->request_handlers_api.setError(socket.error());
        co_return;
    }

    p_tcp_request->request_handlers_api.setStatus(tristan::network::Status::PROCESSED);

    socket.setHost(p_tcp_request->url().hostIP().as_int, p_tcp_request->url().host());
    socket.setPort(p_tcp_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (not socket.connected()) {
        if (p_tcp_request->isPaused()) {
            netInfo("Network request is paused tcp_request->uuid() = " + p_tcp_request->uuid());
            co_return;
        }
        if (p_tcp_request->isCanceled()) {
            netInfo("Network request is cancelled tcp_request->uuid() = " + p_tcp_request->uuid());
            co_return;
        }
        netInfo("Connecting to " + p_tcp_request->url().hostIP().as_string);
        netDebug("tcp_request->uuid() = " + p_tcp_request->uuid());

        socket.connect();

        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_tcp_request)) {
            co_return;
        }
        co_await std::suspend_always();
        socket.resetError();
    }

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = p_tcp_request->requestData().size();

    p_tcp_request->request_handlers_api.setStatus(tristan::network::Status::WRITING);
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (bytes_written < bytes_to_write) {
        if (p_tcp_request->isPaused()) {
            netInfo("Network request is paused tcp_request->uuid() = " + p_tcp_request->uuid());
            co_return;
        }
        if (p_tcp_request->isCanceled()) {
            netInfo("Network request is cancelled tcp_request->uuid() = " + p_tcp_request->uuid());
            co_return;
        }
        netInfo("Writing to " + p_tcp_request->url().hostIP().as_string);
        auto bytes_remain = bytes_to_write - bytes_written;
        uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
        bytes_written += socket.write(p_tcp_request->requestData(), current_frame_size, bytes_written);
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_tcp_request)) {
            co_return;
        }
        co_await std::suspend_always();
        socket.resetError();
    }

    if (p_tcp_request->bytesToRead() != 0) {
        uint64_t bytes_read = 0;
        uint64_t bytes_to_read = p_tcp_request->bytesToRead();
        start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        while (bytes_read < bytes_to_read) {
            if (p_tcp_request->isPaused()) {
                netInfo("Network request is paused tcp_request->uuid() = " + p_tcp_request->uuid());
                co_return;
            }
            if (p_tcp_request->isCanceled()) {
                netInfo("Network request is cancelled tcp_request->uuid() = " + p_tcp_request->uuid());
                co_return;
            }
            auto bytes_remain = bytes_to_read - bytes_read;
            uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
            auto data = socket.read(current_frame_size);

            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_tcp_request)) {
                co_return;
            }
            if (not data.empty()) {
                netDebug("data.size() = " + std::to_string(data.size()));
                netDebug("data = " + std::string(data.begin(), data.end()));
                bytes_read += data.size();
                p_tcp_request->request_handlers_api.addResponseData(std::move(data));
                if (p_tcp_request->error()) {
                    netError(p_tcp_request->error().message());
                    co_return;
                }
            }
            co_await std::suspend_always();
            socket.resetError();
        }
    }
    p_tcp_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
    netInfo("Request " + p_tcp_request->uuid() + " successfully processed");
}

auto tristan::network::private_::AsyncNetworkRequestHandlerImpl::handleHTTPRequest(std::shared_ptr< tristan::network::HttpRequest > p_http_request)
    -> tristan::ResumableCoroutine {
    netInfo("Starting processing of HTTP request " + p_http_request->uuid());

    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(p_http_request);

    tristan::sockets::InetSocket socket;

    if (socket.error()) {
        netError(socket.error().message());
        p_http_request->request_handlers_api.setError(socket.error());
        co_return;
    }

    p_http_request->request_handlers_api.setStatus(tristan::network::Status::PROCESSED);

    socket.setHost(p_http_request->url().hostIP().as_int, p_http_request->url().host());
    socket.setPort(p_http_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (not socket.connected()) {
        if (p_http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
            co_return;
        }
        if (p_http_request->isCanceled()) {
            netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
            co_return;
        }
        netInfo("Connecting to " + p_http_request->url().hostIP().as_string);
        socket.connect(p_http_request->isSSL());

        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
            co_return;
        }
        co_await std::suspend_always();
        socket.resetError();
    }

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = p_http_request->requestData().size();
    p_http_request->request_handlers_api.setStatus(tristan::network::Status::WRITING);
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (bytes_written < bytes_to_write) {
        if (p_http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
            co_return;
        }
        if (p_http_request->isCanceled()) {
            netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
            co_return;
        }
        netInfo("Writing to " + p_http_request->url().hostIP().as_string);
        auto bytes_remain = bytes_to_write - bytes_written;
        uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
        bytes_written += socket.write(p_http_request->requestData(), current_frame_size, bytes_written);
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
            co_return;
        }
        netDebug(std::to_string(current_frame_size) + " bytes was written");
        co_await std::suspend_always();
        socket.resetError();
    }

    p_http_request->request_handlers_api.setStatus(tristan::network::Status::READING);

    std::vector< uint8_t > headers_data;
    start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    while (true) {
        if (p_http_request->isPaused()) {
            netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
            co_return;
        }
        if (p_http_request->isCanceled()) {
            netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
            co_return;
        }
        auto data = socket.readUntil({'\r', '\n', '\r', '\n'});
        if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
            co_return;
        }
        if (socket.error() && socket.error().value() != static_cast< int >(tristan::sockets::Error::READ_DONE)) {
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
        p_http_request->initResponse(std::move(headers_data));
        if (p_http_request->error()) {
            netError(p_http_request->error().message());
            co_return;
        }
        break;
    }

    auto response = std::dynamic_pointer_cast< tristan::network::HttpResponse >(p_http_request->response());
    if (not response) {
        netFatal("Bad dynamic cast");
        std::exit(1);
    }
    if (response->status() != tristan::network::HttpStatus::Ok) {
        p_http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
        co_return;
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
                    co_return;
                }
                if (p_http_request->isCanceled()) {
                    netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
                    co_return;
                }
                auto bytes_remain = bytes_to_read - bytes_read;
                uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);

                auto data = socket.read(current_frame_size);
                if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
                    co_return;
                }
                if (not data.empty()) {
                    netDebug(std::to_string(data.size()) + " bytes was read from " + p_http_request->url().hostIP().as_string);
                    bytes_read += data.size();
                    p_http_request->request_handlers_api.addResponseData(std::move(data));
                    if (p_http_request->error()) {
                        netError(p_http_request->error().message());
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
            p_http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
        }
    } else if (auto transfer_encoding = response->headers()->headerValue(tristan::network::http::header_names::transfer_encoding)) {
        netInfo("Transfer-encoding header found");
        if (transfer_encoding.value().find("chunked") == std::string::npos) {
            netWarning("Transfer-encoding header does not contain chunked specification.");
            p_http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
            co_return;
        }
        while (true) {
            if (p_http_request->isPaused()) {
                netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
                co_return;
            }
            if (p_http_request->isCanceled()) {
                netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
                co_return;
            }
            auto chunk_size = socket.readUntil({'\r', '\n'});
            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
                co_return;
            }
            if (socket.error() && socket.error().value() != static_cast< int >(tristan::sockets::Error::READ_DONE)) {
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
                if (p_http_request->isPaused()) {
                    netInfo("Network request is paused http_request->uuid() = " + p_http_request->uuid());
                    co_return;
                }
                if (p_http_request->isCanceled()) {
                    netInfo("Network request is cancelled http_request->uuid() = " + p_http_request->uuid());
                    co_return;
                }
                auto bytes_remain = bytes_to_read - bytes_read;
                uint8_t current_frame_size = (m_max_frame_size < bytes_remain ? m_max_frame_size : bytes_remain);
                auto data = socket.read(current_frame_size);
                if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
                    co_return;
                }
                if (not data.empty()) {
                    netDebug(std::to_string(data.size()) + " bytes was read from " + p_http_request->url().hostIP().as_string);
                    bytes_read += data.size();
                    p_http_request->request_handlers_api.addResponseData(std::move(data));
                    if (p_http_request->error()) {
                        netError(p_http_request->error().message());
                        co_return;
                    }
                }
                if (socket.error()) {
                    co_await std::suspend_always();
                    socket.resetError();
                }
            }
            auto redundant_data = socket.read(2);
            if (not tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(socket, start, p_http_request)) {
                co_return;
            }
            if (socket.error()) {
                co_await std::suspend_always();
                socket.resetError();
            }
        }
    } else {
        p_http_request->request_handlers_api.setError(tristan::network::makeError(tristan::network::NetworkResponseError::HTTP_RESPONSE_SIZE_ERROR));
        co_return;
    }
    p_http_request->request_handlers_api.setStatus(tristan::network::Status::DONE);
    netInfo("Request " + p_http_request->uuid() + " successfully processed");
}

auto tristan::network::private_::AsyncNetworkRequestHandlerImpl::handleUnimplementedRequest(//NOLINT
    std::shared_ptr< tristan::network::NetworkRequestBase > p_network_request) -> tristan::ResumableCoroutine {
    netError("Unimplemented network request received");
    tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(p_network_request);
    p_network_request->request_handlers_api.setStatus(tristan::network::Status::ERROR);
    p_network_request->request_handlers_api.setError(tristan::network::makeError(tristan::network::ErrorCode::REQUEST_NOT_SUPPORTED));
    co_return;
}
