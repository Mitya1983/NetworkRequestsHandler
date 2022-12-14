#include "http_response.hpp"
#include "network_error.hpp"
tristan::network::HttpResponse::HttpResponse(std::string&& uuid, std::vector< uint8_t >&& headers_data) :
    tristan::network::NetworkResponse(std::move(uuid)) {
    if (std::string(headers_data.begin(), headers_data.begin() + 8) != "HTTP/1.1") {
        m_error = tristan::network::makeError(tristan::network::NetworkResponseError::HTTP_BAD_RESPONSE_FORMAT);
        return;
    }
    m_status = static_cast<tristan::network::HttpStatus>(std::stoi(std::string(headers_data.begin() + 9, headers_data.begin() + 12)));
    m_response_headers = std::make_unique< tristan::network::HttpHeaders >(std::move(headers_data));
}

auto tristan::network::HttpResponse::createResponse(std::string uuid, std::vector< uint8_t >&& headers_data) -> std::shared_ptr< HttpResponse > {
    return std::shared_ptr< tristan::network::HttpResponse >(new tristan::network::HttpResponse(std::move(uuid), std::move(headers_data)));
}

auto tristan::network::HttpResponse::error() const -> std::error_code { return m_error; }

auto tristan::network::HttpResponse::status() const -> tristan::network::HttpStatus { return m_status; }

auto tristan::network::HttpResponse::headers() const -> const std::unique_ptr< HttpHeaders >& { return m_response_headers; }
