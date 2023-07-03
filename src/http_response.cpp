#include "http_response.hpp"
#include "network_error.hpp"
tristan::network::HttpResponse::HttpResponse(std::string&& p_string, std::vector< uint8_t >&& p_headers_data) :
    tristan::network::NetworkResponse(std::move(p_string)) {
    if (std::string(p_headers_data.begin(), p_headers_data.begin() + 8) != "HTTP/1.1") {
        m_error = tristan::network::makeError(tristan::network::NetworkResponseError::HTTP_BAD_RESPONSE_FORMAT);
        return;
    }
    m_status = static_cast<tristan::network::HttpStatus>(std::stoi(std::string(p_headers_data.begin() + 9, p_headers_data.begin() + 12)));
    m_response_headers = std::make_unique< tristan::network::HttpHeaders >(std::move(p_headers_data));
}

auto tristan::network::HttpResponse::createResponse(std::string p_uuid, std::vector< uint8_t >&& p_headers_data) -> std::shared_ptr< HttpResponse > {
    return std::shared_ptr< tristan::network::HttpResponse >(new tristan::network::HttpResponse(std::move(p_uuid), std::move(p_headers_data)));
}

auto tristan::network::HttpResponse::error() const -> std::error_code { return m_error; }

auto tristan::network::HttpResponse::status() const -> tristan::network::HttpStatus { return m_status; }

auto tristan::network::HttpResponse::headers() const -> const std::unique_ptr< HttpHeaders >& { return m_response_headers; }
