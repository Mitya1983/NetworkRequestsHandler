#include "network_request.hpp"

tristan::network::NetworkRequest::NetworkRequest(tristan::network::Url&& url) :
    tristan::network::NetworkRequestPublicAPI(std::move(url)) { }

tristan::network::NetworkRequest::NetworkRequest(const tristan::network::Url& url) :
    NetworkRequest(Url(url)) { }

auto tristan::network::NetworkRequest::requestData() -> const std::vector< uint8_t >& { return m_request_data; }
