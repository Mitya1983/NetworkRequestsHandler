#include "network_response.hpp"

tristan::network::NetworkResponse::NetworkResponse(std::string&& p_uuid) :
    m_uuid(p_uuid) { }

auto tristan::network::NetworkResponse::createResponse(std::string p_uuid) -> std::shared_ptr< NetworkResponse > {
    return std::shared_ptr< tristan::network::NetworkResponse >(new tristan::network::NetworkResponse(std::move(p_uuid)));
}

auto tristan::network::NetworkResponse::uuid() const noexcept -> const std::string& { return m_uuid; }

auto tristan::network::NetworkResponse::data() const -> std::shared_ptr< std::vector< uint8_t > > { return m_response_data; }
