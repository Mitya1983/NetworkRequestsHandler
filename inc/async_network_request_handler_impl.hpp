#ifndef ASYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP
#define ASYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP

#include "network_request_handler_impl.hpp"
#include "resumable_coroutine.hpp"
namespace tristan::network::private_ {

    class AsyncNetworkRequestHandlerImpl : public NetworkRequestHandlerImpl {
    public:
        auto handleRequest(std::shared_ptr< NetworkRequestBase >&& network_request)  -> tristan::network::ResumableCoroutine;
        ~AsyncNetworkRequestHandlerImpl() override = default;

    protected:
        auto processTcpRequest(std::shared_ptr< tristan::network::TcpRequest > tcp_request) -> tristan::network::ResumableCoroutine;
        auto processHTTPRequest(std::shared_ptr< tristan::network::HttpRequest > http_request) -> tristan::network::ResumableCoroutine;
        auto processUnimplementedRequest() -> tristan::network::ResumableCoroutine;
    };

}  // namespace tristan::network::private_

#endif  //ASYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP

