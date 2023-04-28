#ifndef ASYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP
#define ASYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP

#include "network_request_handler_impl.hpp"

#include <resumable_coroutine.hpp>

namespace tristan::network::private_ {

    class AsyncNetworkRequestHandlerImpl : public NetworkRequestHandlerImpl {
    public:
        AsyncNetworkRequestHandlerImpl();
        ~AsyncNetworkRequestHandlerImpl() override;
        auto handleRequest(std::shared_ptr< NetworkRequestBase >&& network_request) -> tristan::ResumableCoroutine;

    protected:
        auto handleTcpRequest(std::shared_ptr< tristan::network::TcpRequest > tcp_request) -> tristan::ResumableCoroutine;
        auto handleHTTPRequest(std::shared_ptr< tristan::network::HttpRequest > http_request) -> tristan::ResumableCoroutine;
        auto handleUnimplementedRequest(std::shared_ptr< tristan::network::NetworkRequestBase > network_request) -> tristan::ResumableCoroutine;
    };

}  // namespace tristan::network::private_

#endif  //ASYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP
