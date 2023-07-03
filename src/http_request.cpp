#include "http_request.hpp"
#include "http_header_names.hpp"
#include "http_response.hpp"
#include "network_utility.hpp"
#include "network_logger.hpp"

//#include "asio/io_context.hpp"
//#include "asio/connect.hpp"
//#include "asio/read.hpp"
//#include "asio/ssl/error.hpp"
//#include "asio/ip/tcp.hpp"
//#include "asio/ssl/stream.hpp"
//
//#include <thread>
//#include <array>
//#include <fstream>
//#include <utility>

tristan::network::HttpRequest::HttpRequest(Url&& url) :
    NetworkRequestBase(std::move(url)),
    m_request_composed(false) {
    if (not m_url.isValid() || (m_url.scheme() != "http" && m_url.scheme() != "https" && m_url.port() != "80" && m_url.port() != "443")) {
        netError("Invalid url for HTTP request received");
        tristan::network::NetworkRequestBase::setError(tristan::network::makeError(tristan::network::ErrorCode::INVALID_URL));
        return;
    }
    m_headers.addHeader(tristan::network::Header(tristan::network::http::header_names::host, m_url.host()));
    if (m_url.portUint16_t_local_byte_order() == 443) {
        m_ssl = true;
    }
}

tristan::network::HttpRequest::HttpRequest(const tristan::network::Url& url) :
    HttpRequest(Url(url)) { }

void tristan::network::HttpRequest::addHeader(tristan::network::Header&& header) {
    if (header.m_name.empty()) {
        return;
    }
    m_headers.addHeader(std::move(header));
}

void tristan::network::HttpRequest::addParam(tristan::network::Parameter&& parameter) {
    if (parameter.m_name.empty()) {
        return;
    }
    m_params.addParameter(std::move(parameter));
}

void tristan::network::HttpRequest::initResponse(std::vector< uint8_t >&& headers_data) {
    m_response = tristan::network::HttpResponse::createResponse(m_uuid, std::move(headers_data));
}

tristan::network::GetRequest::GetRequest(Url&& url) :
    HttpRequest(std::move(url)) { }

tristan::network::GetRequest::GetRequest(const tristan::network::Url& url) :
    HttpRequest(Url(url)) { }

auto tristan::network::GetRequest::requestData() -> const std::vector< uint8_t >& {
    if (not m_request_composed) {
        std::string to_insert("GET ");
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());
        if (m_url.path().empty() || m_url.path().at(0) != '/') {
            m_request_data.push_back('/');
        }
        m_request_data.insert(m_request_data.end(), m_url.path().begin(), m_url.path().end());
        if (!m_params.empty()) {
            m_request_data.push_back('?');
            int param_count = 0;
            for (const auto& param: m_params) {
                if (param_count > 0) {
                    m_request_data.push_back('&');
                }
                m_request_data.insert(m_request_data.end(), param.m_name.begin(), param.m_name.end());
                if (!param.m_string.empty()) {
                    m_request_data.push_back('=');
                    m_request_data.insert(m_request_data.end(), param.m_string.begin(), param.m_string.end());
                }
                ++param_count;
            }
        }
        if (!m_url.query().empty()) {
            m_request_data.push_back('&');
            m_request_data.insert(m_request_data.end(), m_url.query().begin(), m_url.query().end());
        }
        to_insert = " HTTP/1.1\r\n";
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());
        if (!m_headers.empty()) {
            for (const auto& header: m_headers) {
                m_request_data.insert(m_request_data.end(), header.m_name.begin(), header.m_name.end());
                m_request_data.push_back(':');
                m_request_data.insert(m_request_data.end(), header.m_string.begin(), header.m_string.end());
                m_request_data.push_back('\r');
                m_request_data.push_back('\n');
            }
        }
        m_request_data.push_back('\r');
        m_request_data.push_back('\n');
        m_request_data.shrink_to_fit();
        m_request_composed = true;
    }
    return m_request_data;
}

tristan::network::PostRequest::PostRequest(Url&& url) :
    HttpRequest(std::move(url)) { }

tristan::network::PostRequest::PostRequest(const tristan::network::Url& url) :
    HttpRequest(Url(url)) { }

void tristan::network::PostRequest::setBody(std::string&& p_body) {
    m_body = std::move(p_body);
}

void tristan::network::PostRequest::setBody(const std::string& p_body) {
    m_body = p_body;
}

auto tristan::network::PostRequest::requestData() -> const std::vector< uint8_t >& {
    if (not m_request_composed) {
        if (m_body.empty() and not m_params.empty()) {
            int param_count = 0;
            for (const auto& param: m_params) {
                if (param_count > 0) {
                    m_body += '&';
                }
                m_body += param.m_name;
                m_body += '=';
                auto content_type = m_headers.headerValue(tristan::network::http::header_names::content_type);
                if (content_type && content_type.value() == "application/x-www-form-urlencoded") {
                    m_body += tristan::network::utility::encodeUrl(param.m_string);
                } else if (content_type && content_type.value() == "multipart/form-data") {
                    //NOTE: To be developed in following versions
                } else {
                    m_body += param.m_string;
                }
                ++param_count;
            }
        }
        m_headers.addHeader(tristan::network::Header(tristan::network::http::header_names::content_length, std::to_string(m_body.size())));
        std::string to_insert = "POST ";
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());
        if (m_url.path().empty() || m_url.path().at(0) != '/') {
            m_request_data.push_back('/');
        }
        m_request_data.insert(m_request_data.end(), m_url.path().begin(), m_url.path().end());
        to_insert = " HTTP/1.1\r\n";
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());

        if (!m_headers.empty()) {
            for (const auto& header: m_headers) {
                m_request_data.insert(m_request_data.end(), header.m_name.begin(), header.m_name.end());
                m_request_data.push_back(':');
                m_request_data.insert(m_request_data.end(), header.m_string.begin(), header.m_string.end());
                m_request_data.push_back('\r');
                m_request_data.push_back('\n');
            }
        }

        m_request_data.push_back('\r');
        m_request_data.push_back('\n');

        if (not m_body.empty()) {
            m_request_data.insert(m_request_data.end(), m_body.begin(), m_body.end());
        }
        m_request_data.shrink_to_fit();
        m_request_composed = true;
    }
    return m_request_data;
}

tristan::network::PutRequest::PutRequest(tristan::network::Url&& url) :
    PostRequest(std::move(url)) { }

tristan::network::PutRequest::PutRequest(const tristan::network::Url& url) :
    PostRequest(url) { }

auto tristan::network::PutRequest::requestData() -> const std::vector< uint8_t >& {
    if (not m_request_composed) {
        if (m_body.empty() and not m_params.empty()) {
            int param_count = 0;
            for (const auto& param: m_params) {
                if (param_count > 0) {
                    m_body += '&';
                }
                m_body += param.m_name;
                m_body += '=';
                auto content_type = m_headers.headerValue(tristan::network::http::header_names::content_type);
                if (content_type && content_type.value() == "application/x-www-form-urlencoded") {
                    m_body += tristan::network::utility::encodeUrl(param.m_string);
                } else if (content_type && content_type.value() == "multipart/form-data") {
                    //NOTE: To be developed in following versions
                } else {
                    m_body += param.m_string;
                }
                ++param_count;
            }
        }
        m_headers.addHeader(tristan::network::Header(tristan::network::http::header_names::content_length, std::to_string(m_body.size())));
        std::string to_insert = "PUT ";
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());
        if (m_url.path().empty() || m_url.path().at(0) != '/') {
            m_request_data.push_back('/');
        }
        m_request_data.insert(m_request_data.end(), m_url.path().begin(), m_url.path().end());
        to_insert = " HTTP/1.1\r\n";
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());

        if (!m_headers.empty()) {
            for (const auto& header: m_headers) {
                m_request_data.insert(m_request_data.end(), header.m_name.begin(), header.m_name.end());
                m_request_data.push_back(':');
                m_request_data.insert(m_request_data.end(), header.m_string.begin(), header.m_string.end());
                m_request_data.push_back('\r');
                m_request_data.push_back('\n');
            }
        }

        m_request_data.push_back('\r');
        m_request_data.push_back('\n');

        if (not m_body.empty()) {
            m_request_data.insert(m_request_data.end(), m_body.begin(), m_body.end());
        }
        m_request_data.shrink_to_fit();
        m_request_composed = true;
    }
    return m_request_data;
}
