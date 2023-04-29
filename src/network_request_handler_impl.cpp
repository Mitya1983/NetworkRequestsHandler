#include "network_request_handler_impl.hpp"
#include "network_logger.hpp"

#include <socket_error.hpp>

tristan::network::private_::NetworkRequestHandlerImpl::NetworkRequestHandlerImpl() = default;

tristan::network::private_::NetworkRequestHandlerImpl::~NetworkRequestHandlerImpl() = default;

void tristan::network::private_::NetworkRequestHandlerImpl::debugNetworkRequestInfo(const std::shared_ptr< NetworkRequestBase >& network_request) {
    netDebug("network_request->uuid() = " + network_request->uuid());
    netDebug("network_request->url().hostIP().as_string = " + network_request->url().hostIP().as_string);
    netDebug("network_request->url().port() = " + network_request->url().port());
    netDebug("network_request->url() = " + network_request->url().composeUrl());
    netDebug("network_request->requestData() = " + std::string(network_request->requestData().begin(), network_request->requestData().end()));
    netDebug("network_request->requestData().size() = " + std::to_string(network_request->requestData().size()));
    netDebug("network_request->bytesToRead() = " + std::to_string(network_request->bytesToRead()));
    netDebug("network_request->responseDelimiter() = " + std::string(network_request->responseDelimiter().begin(), network_request->responseDelimiter().end()));
}

bool tristan::network::private_::NetworkRequestHandlerImpl::checkSocketOperationErrorAndTimeOut(
    const tristan::sockets::InetSocket& socket,
    std::chrono::time_point< std::chrono::system_clock, std::chrono::microseconds > time_point,
    const std::shared_ptr<NetworkRequestBase>& network_request) {
    if (socket.error() && socket.error().value() != static_cast< int >(tristan::sockets::Error::CONNECT_TRY_AGAIN)
        && socket.error().value() != static_cast< int >(tristan::sockets::Error::CONNECT_IN_PROGRESS)
        && socket.error().value() != static_cast< int >(tristan::sockets::Error::CONNECT_ALREADY_IN_PROCESS)
        && socket.error().value() != static_cast< int >(tristan::sockets::Error::WRITE_TRY_AGAIN)
        && socket.error().value() != static_cast< int >(tristan::sockets::Error::READ_TRY_AGAIN)
        && socket.error().value() != static_cast< int >(tristan::sockets::Error::READ_DONE)) {
        netError(socket.error().message());
        netDebug("socket.error().value() = " + std::to_string(socket.error().value()));
        network_request->request_handlers_api.setError(socket.error());
        return false;
    }
    auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
    if (std::chrono::duration_cast< std::chrono::seconds >(end - time_point) >= network_request->timeout()) {
        netError(socket.error().message());
        network_request->request_handlers_api.setError(tristan::sockets::makeError(tristan::sockets::Error::SOCKET_TIMED_OUT));
        return false;
    }
    return true;
}
