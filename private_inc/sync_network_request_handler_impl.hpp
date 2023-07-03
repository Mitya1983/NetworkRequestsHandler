#ifndef SYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP
#define SYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP

#include "network_request_handler_impl.hpp"

namespace tristan::network::private_ {

    class SyncNetworkRequestHandlerImpl : public NetworkRequestHandlerImpl{
    public:
        SyncNetworkRequestHandlerImpl();
        ~SyncNetworkRequestHandlerImpl() override;
        void handleRequest(std::shared_ptr<NetworkRequestBase>&& p_network_request);
    protected:
        void handleTcpRequest(std::shared_ptr<TcpRequest>&& p_tcp_request);
        void handleHttpRequest(std::shared_ptr<HttpRequest>&& p_http_request);
        void handleUnimplementedRequest(std::shared_ptr< tristan::network::NetworkRequestBase >&& p_network_request);
    private:
        std::chrono::milliseconds m_sleeping_interval = std::chrono::milliseconds(250);
        const uint16_t m_max_frame_size = std::numeric_limits<uint16_t>::max();
    };

} //End of tristan::network::private_ namespace

#endif  //SYNC_NETWORK_REQUEST_HANDLER_IMPL_HPP
