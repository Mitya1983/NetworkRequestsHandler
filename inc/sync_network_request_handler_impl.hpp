#ifndef SYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP
#define SYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP

#include "network_request_handler_impl.hpp"

namespace tristan::network::private_ {

    class SyncNetworkRequestHandlerImpl : public NetworkRequestHandlerImpl{
    public:
        void handleRequest(std::shared_ptr<NetworkRequestBase>&& network_request);
        ~SyncNetworkRequestHandlerImpl() override = default;
    protected:
        void handleTcpRequest(std::shared_ptr<TcpRequest>&& tcp_request);
        void handleHttpRequest(std::shared_ptr<HttpRequest>&& http_request);
        void handleUnimplementedRequest(std::shared_ptr< tristan::network::NetworkRequestBase >&& network_request);
    private:
        std::chrono::milliseconds m_sleeping_interval = std::chrono::milliseconds(250);
    };

} //End of tristan::network::private_ namespace

#endif  //SYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP
