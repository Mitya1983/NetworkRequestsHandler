#include "httprequest.hpp"

namespace {

std::string httpResponseCode(const std::string &httpResponse);
std::string httpResponseDescription(const std::string &httpResponse);

} // namespace name

network::Request::Request(const std::string &url, const std::string &descritpion) :
    m_description(descritpion),
    m_host(_get_host_from_url(url)),
    m_requestUrl(_get_request_from_url(url)),
    m_bytesToRead(0),
    m_bytesRead(0)
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

    if (end == std::string::npos){
        return "/";
    }

    return url.substr(end);
}

void network::Request::_parse_reply_base()
{
    if(m_reply_base.empty()){
        return;
    }
    m_httpStatus = static_cast<HttpStatus>(std::stoi(httpResponseCode(m_reply_base)));
    if (m_httpStatus > HttpStatus::IM_Used && m_httpStatus < HttpStatus::Bad_Request){
        m_status = Status::REDIRECT;
    }
    else if (m_httpStatus >= HttpStatus::Bad_Request){
        m_status = Status::ERROR;
    }
    m_errorDescription = network::getHttpStatusDetails(m_httpStatus);
    m_headers.clear();
    size_t start = m_reply_base.find("\r\n"), end = 0;
    while (true){
        start = m_reply_base.find_first_not_of(" \r\n\t", start);
        if (start == std::string::npos){
            break;
        }
        end = m_reply_base.find(": ", start);
        if (end == std::string::npos){
            break;
        }
        std::string header = m_reply_base.substr(start, end - start);
        for (auto &c : header){
            c = std::tolower(c);
        }
        start = end + 2;
        end = m_reply_base.find("\r\n", start);
        std::string value = m_reply_base.substr(start, end - start);
        start = end;
        m_headers.emplace_back(header, value);
    }
}

void network::Request::add_header(const std::string &header, const std::string &value)
{
    m_headers.emplace_back(header, value);
}

void network::Request::add_param(const std::string &paramName, const std::string &paramValue)
{
    m_params.emplace(paramName, paramValue);
}

std::vector<std::pair<std::string, std::string>>::const_iterator network::Request::find(const std::string headerName) const
{
    size_t count = 0;
    for (const auto &header : m_headers){
        if (header.first == headerName){
            break;
        }
        ++count;
    }
    if (count < m_headers.size()){
        return m_headers.cbegin() + count;
    }

    return m_headers.cend();
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
            if (!param.second.empty()){
                l_request += '=';
                l_request += param.second;
            }
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

namespace {

std::string httpResponseCode(const std::string & httpResponse){
    return httpResponse.substr(9, 3);
}
std::string httpResponseDescription(const std::string & httpResponse){
    auto endPos = httpResponse.find("\r\n");
    return httpResponse.substr(13, endPos - 13);
}
} // namespace name
