#include "network_response.hpp"

tristan::network::NetworkResponse::NetworkResponse(std::string&& uuid) :
    m_uuid(uuid) { }

auto tristan::network::NetworkResponse::createResponse(std::string uuid) -> std::shared_ptr< NetworkResponse > {
    return std::shared_ptr< tristan::network::NetworkResponse >(new tristan::network::NetworkResponse(std::move(uuid)));
}

auto tristan::network::NetworkResponse::uuid() const noexcept -> const std::string& { return m_uuid; }

auto tristan::network::NetworkResponse::data() const -> std::shared_ptr< std::vector< uint8_t > > { return m_response_data; }
