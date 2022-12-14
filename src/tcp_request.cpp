#include "tcp_request.hpp"

tristan::network::TcpRequest::TcpRequest(tristan::network::Url&& url) :
    tristan::network::NetworkRequestBase(std::move(url)) { }

tristan::network::TcpRequest::TcpRequest(const tristan::network::Url& url) :
    TcpRequest(Url(url)) { }

auto tristan::network::TcpRequest::requestData() -> const std::vector< uint8_t >& { return m_request_data; }
