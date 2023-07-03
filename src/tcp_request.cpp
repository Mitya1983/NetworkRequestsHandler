#include "tcp_request.hpp"

tristan::network::TcpRequest::TcpRequest(tristan::network::Url&& p_url) :
    tristan::network::NetworkRequestBase(std::move(p_url)) { }

tristan::network::TcpRequest::TcpRequest(const tristan::network::Url& p_url) :
    TcpRequest(Url(p_url)) { }

auto tristan::network::TcpRequest::requestData() -> const std::vector< uint8_t >& { return m_request_data; }
