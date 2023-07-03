#ifndef NETWORK_REQUEST_HANDLER_IMPL_HPP
#define NETWORK_REQUEST_HANDLER_IMPL_HPP

#include "tcp_request.hpp"
#include "http_request.hpp"
#include "inet_socket.hpp"

#include <chrono>

namespace tristan::network::private_ {

    class NetworkRequestHandlerImpl {
    public:
        NetworkRequestHandlerImpl();

        virtual ~NetworkRequestHandlerImpl();
    protected:
        static void debugNetworkRequestInfo(const std::shared_ptr< NetworkRequestBase >& p_network_request);
        [[nodiscard]] static bool
            checkSocketOperationErrorAndTimeOut(const tristan::sockets::InetSocket& p_socket,
                                                std::chrono::time_point< std::chrono::system_clock, std::chrono::microseconds > p_time_point,
                                                const std::shared_ptr< NetworkRequestBase >& p_network_request);
    };

}  // namespace tristan::network::private_

#endif  //NETWORK_REQUEST_HANDLER_IMPL_HPP
