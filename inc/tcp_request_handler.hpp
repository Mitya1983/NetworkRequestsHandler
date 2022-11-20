#ifndef TCP_REQUEST_HANDLER_HPP
#define TCP_REQUEST_HANDLER_HPP

#include "network_request.hpp"
#include "socket.hpp"

namespace tristan::network {

    class TcpRequestHandler
    {
    public:
        explicit TcpRequestHandler(std::chrono::seconds time_out_interval = std::chrono::seconds(10));
        TcpRequestHandler(const TcpRequestHandler& other) = default;
        TcpRequestHandler(TcpRequestHandler&& other) = default;

        TcpRequestHandler& operator=(const TcpRequestHandler& other) = default;
        TcpRequestHandler& operator=(TcpRequestHandler&& other) = default;

        virtual ~TcpRequestHandler() = default;

        virtual void processRequest(std::shared_ptr< tristan::network::NetworkRequest >&& network_request);

    protected:
        std::chrono::seconds m_time_out_interval;
    private:
    };

} //End of tristan::network namespace

#endif  //TCP_REQUEST_HANDLER_HPP
