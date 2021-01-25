#include "httprequest.hpp"

network::Request::Request(const std::string &url, const std::string &descritpion) :
    m_description(descritpion),
    m_host(_get_host_from_url(url)),
    m_requestUrl(_get_request_from_url(url))
{
    m_headers.emplace_back("Host", m_host);
    m_headers.emplace_back("Connection", "close");
}

std::string network::Request::_get_host_from_url(const std::string &url)
{
    auto start = url.find("://");
    start += 3;
    auto end = url.find_first_of('/', start);

    return url.substr(start, end - start);
}

std::string network::Request::_get_request_from_url(const std::string &url)
{
    auto end = url.find_first_of('/', 8);

    return url.substr(end);
}

void network::Request::add_header(const std::string &header, const std::string &value)
{
    m_headers.emplace_back(header, value);
}

void network::Request::add_param(std::string paramName, const std::string &paramValue)
{
    m_params.emplace(paramName, paramValue);
}

network::GetRequest::GetRequest(const std::string &url, const std::string &descritpion) :
    Request(url, descritpion)
{

}

std::string network::GetRequest::request() const
{
    std::string l_request = "GET ";
    l_request += m_requestUrl;
    if (!m_params.empty()){
        l_request += '?';
        int paramCount = 0;
        for (const auto &param : m_params){
            if (paramCount > 0){
                l_request += '&';
            }
            l_request += param.first;
            l_request += '=';
            l_request += param.second;
            ++paramCount;
        }
    }
    l_request += " HTTP/1.1\r\n";
    if (!m_headers.empty()){
        for (const auto &header : m_headers){
            l_request += header.first;
            l_request += ": ";
            l_request += header.second;
            l_request += "\r\n";
        }
    }
    l_request += "\r\n\r\n";

    return l_request;
}

std::string network::PostRequest::request() const
{
    std::string l_request = "POST /";
    l_request += m_requestUrl;
    l_request += " HTTP/1.1\r\n";
    if (!m_headers.empty()){
        for (const auto &header : m_headers){
            l_request += header.first;
            l_request += ": ";
            l_request += header.second;
            l_request += "\r\n";
        }
    }
    l_request += "\r\n";
    if (!m_params.empty()){
        int paramCount = 0;
        for (const auto &param : m_params){
            if (paramCount > 0){
                l_request += '&';
            }
            l_request += param.first;
            l_request += '=';
            l_request += param.second;
            ++paramCount;
        }
    }
    l_request += "\r\n\r\n";

    return l_request;
}
