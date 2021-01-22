#ifndef NETWORK_REQUESTS_HPP
#define NETWORK_REQUESTS_HPP

#include "httprequest.hpp"

#include <memory>
namespace network {

void process_ssl_network_request(std::shared_ptr<Request> request);

} // namespace zestad::network

#endif // NETWORK_REQUESTS_HPP
