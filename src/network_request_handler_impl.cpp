#include "network_request_handler_impl.hpp"
#include "network_logger.hpp"

#include <socket_error.hpp>

tristan::network::private_::NetworkRequestHandlerImpl::NetworkRequestHandlerImpl() = default;

tristan::network::private_::NetworkRequestHandlerImpl::~NetworkRequestHandlerImpl() = default;

void tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(const std::shared_ptr< NetworkRequestBase >& p_network_request) {
    netDebug("network_request->uuid() = " + p_network_request->uuid());
    netDebug("network_request->url().hostIP().as_string = " + p_network_request->url().hostIP().as_string);
    netDebug("network_request->url().port() = " + p_network_request->url().port());
    netDebug("network_request->url() = " + p_network_request->url().composeUrl());
    netDebug("network_request->requestData() = " + std::string(p_network_request->requestData().begin(), p_network_request->requestData().end()));
    netDebug("network_request->requestData().size() = " + std::to_string(p_network_request->requestData().size()));
    netDebug("network_request->bytesToRead() = " + std::to_string(p_network_request->bytesToRead()));
    netDebug("network_request->responseDelimiter() = " + std::string(p_network_request->responseDelimiter().begin(), p_network_request->responseDelimiter().end()));
}

bool tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(
    const tristan::sockets::InetSocket& p_socket,
    std::chrono::time_point< std::chrono::system_clock, std::chrono::microseconds > p_time_point,
    const std::shared_ptr<NetworkRequestBase>& p_network_request) {
    if (p_socket.error() && p_socket.error().value() != static_cast< int >(tristan::sockets::Error::CONNECT_TRY_AGAIN)
        && p_socket.error().value() != static_cast< int >(tristan::sockets::Error::CONNECT_IN_PROGRESS)
        && p_socket.error().value() != static_cast< int >(tristan::sockets::Error::CONNECT_ALREADY_IN_PROCESS)
        && p_socket.error().value() != static_cast< int >(tristan::sockets::Error::WRITE_TRY_AGAIN)
        && p_socket.error().value() != static_cast< int >(tristan::sockets::Error::READ_TRY_AGAIN)
        && p_socket.error().value() != static_cast< int >(tristan::sockets::Error::READ_DONE)) {
        netError(p_socket.error().message());
        netDebug("socket.error().value() = " + std::to_string(p_socket.error().value()));
        p_network_request->request_handlers_api.setError(p_socket.error());
        return false;
    }
    auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    if (std::chrono::duration_cast< std::chrono::seconds >(end - p_time_point) >= p_network_request->timeout()) {
        netError(p_socket.error().message());
        p_network_request->request_handlers_api.setError(tristan::sockets::makeError(tristan::sockets::Error::SOCKET_TIMED_OUT));
        return false;
    }
    return true;
}
