#include "http_header.hpp"
#include "net_log.hpp"

#include <algorithm>

tristan::network::HttpHeaders::HttpHeaders(const std::string& headers_data) {
    netTrace("Start");
    netDebug("Parsing http headers: " + headers_data);
    size_t new_line_start = 0;

    while (true) {
        if (new_line_start >= headers_data.size() - 1) {
            break;
        }
        size_t new_line_end = headers_data.find("\r\n", new_line_start);
        if (new_line_end == std::string::npos){
            new_line_end = headers_data.size();
        }
        auto line = headers_data.substr(new_line_start, new_line_end - new_line_start);
        if (line.find("HTTP/") != std::string::npos) {
            netDebug("Http base bar was received");
            continue;
        }
        auto header_name_end = line.find(':');
        if (header_name_end == std::string::npos) {
            netError("HTTP header format not conform: " + line);
            continue;
        }
        auto header_name = line.substr(0, header_name_end);
        for (auto& character: header_name) {
            character = static_cast< char >(std::tolower(character));
        }
        auto header_value_start = header_name_end + 1;
        if (line.at(header_value_start) == ' ') {
            ++header_value_start;
        }
        auto header_value = line.substr(header_value_start, line.size() - header_value_start);
        m_headers.emplace_back(std::move(header_name), std::move(header_value));
        new_line_start = new_line_end + 2;
    }
    netTrace("End");
}

tristan::network::HttpHeaders::HttpHeaders(const std::vector< uint8_t >& headers_data) :
    tristan::network::HttpHeaders(std::string(headers_data.begin(), headers_data.end())) { }

auto tristan::network::HttpHeaders::headerValue(const std::string& header_name) const -> std::optional< std::string > {
    auto found = std::find_if(m_headers.begin(), m_headers.end(), [header_name](const tristan::network::Header& header) -> bool {
        return header.name == header_name;
    });
    if (found == m_headers.cend()) {
        return std::nullopt;
    }
    return found->value;
}

void tristan::network::HttpHeaders::addHeader(tristan::network::Header&& header) { m_headers.emplace_back(std::move(header)); }

auto tristan::network::HttpHeaders::empty() -> bool { return m_headers.empty(); }
