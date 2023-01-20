#ifndef NETWORK_REQUEST_HANDLER_IMPL_HPP
#define NETWORK_REQUEST_HANDLER_IMPL_HPP

#include "tcp_request.hpp"
#include "http_request.hpp"
#include "inet_socket.hpp"

#include <chrono>

namespace tristan::network::private_ {

    class NetworkRequestHandlerImpl {
    public:
        virtual ~NetworkRequestHandlerImpl() = default;
    protected:
        static void debugNetworkRequestInfo(const std::shared_ptr< NetworkRequestBase >& network_request);
        [[nodiscard]] static bool
            checkSocketOperationErrorAndTimeOut(const tristan::sockets::InetSocket& socket,
                                                std::chrono::time_point< std::chrono::system_clock, std::chrono::microseconds > time_point,
                                                const std::shared_ptr< NetworkRequestBase >& network_request);

        uint8_t m_max_frame_size = std::numeric_limits<uint8_t>::max();
    };

}  // namespace tristan::network::private_

#endif  //NETWORK_REQUEST_HANDLER_IMPL_HPP
