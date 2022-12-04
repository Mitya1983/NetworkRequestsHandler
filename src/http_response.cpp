#include "http_response.hpp"

tristan::network::HttpResponse::HttpResponse(std::string&& uuid, std::vector<uint8_t>&& headers_data) :
    tristan::network::NetworkResponse(std::move(uuid)) {
    m_response_headers = std::make_unique<tristan::network::HttpHeaders>(std::move(headers_data));
}

auto tristan::network::HttpResponse::createResponse(std::string uuid, std::vector<uint8_t>&& headers_data) -> std::shared_ptr< HttpResponse > {
    return std::shared_ptr< tristan::network::HttpResponse >(new tristan::network::HttpResponse(std::move(uuid), std::move(headers_data)));
}

auto tristan::network::HttpResponse::headers() const -> const std::unique_ptr< HttpHeaders >& { return m_response_headers; }
