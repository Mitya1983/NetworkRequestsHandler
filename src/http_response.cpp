#include "http_response.hpp"

tristan::network::HttpResponse::HttpResponse(const std::string& response_base, std::string uuid) :
        NetworkResponse(std::move(uuid)){

    auto status_start = response_base.find(' ') + 1;
    auto status_end = response_base.find(' ', status_start);

    m_status = static_cast<tristan::network::HttpStatus>(static_cast<uint16_t>(std::stoi(response_base.substr(status_start, status_end - status_start))));

    auto new_line_start = response_base.find("\r\n", status_end) + 2;
    while (true){
        auto header_end = response_base.find(':', new_line_start);
        if (header_end == std::string::npos){
            break;
        }
        auto header = response_base.substr(new_line_start, header_end - new_line_start);
        for (auto& character: header){
            character = static_cast<char>(std::tolower(character));
        }
        auto value_start = header_end + 1;
        if (response_base.at(value_start) == ' '){
            ++value_start;
        }
        auto value_end = response_base.find("\r\n", value_start);
        auto value = response_base.substr(value_start, value_end - value_start);
        m_headers.emplace(header, value);
        new_line_start = value_end + 2;
    }
}

std::optional<std::string> tristan::network::HttpResponse::headerValue(const std::string& header_name) const{
    auto header = m_headers.find(header_name);
    if (header == m_headers.cend()){
        return { };
    }
    return header->second;
}

auto tristan::network::HttpResponse::statusDescription() const -> const std::string&{
    return tristan::network::getHttpStatusDetails(m_status);
}