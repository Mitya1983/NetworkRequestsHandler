#include "network_response.hpp"

tristan::network::NetworkResponse::NetworkResponse(std::string uuid) :
        m_uuid(std::move(uuid)){

}

void tristan::network::NetworkResponse::setResponseData(std::shared_ptr<std::vector<uint8_t>> response_data){
    m_response_data = std::move(response_data);
}
