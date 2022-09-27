#include "url.hpp"
#include "network_utility.hpp"
#include "network_schemes.hpp"
#include <netdb.h>

#include <unordered_map>
#include <regex>

namespace {

    inline const std::unordered_map< char, std::string > g_percentage_encoding = {
        {' ',  "%20"},
        {'!',  "%21"},
        {'@',  "%40"},
        {'#',  "%23"},
        {'$',  "%24"},
        {'%',  "%25"},
        {'&',  "%26"},
        {'*',  "%2A"},
        {'(',  "%28"},
        {')',  "%29"},
        {'+',  "%2B"},
        {'=',  "%3D"},
        {'[',  "%5B"},
        {']',  "%5D"},
        {':',  "%3A"},
        {';',  "%3B"},
        {'\'', "%27"},
        {',',  "%2C"},
        {'/',  "%2F"},
        {'?',  "%3F"},
    };

    constexpr const char* g_ip_regex_pattern = "^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$";

}  //End of unnamed namespace

tristan::network::Url::Url() noexcept :
    m_port_local_byte_order(0),
    m_port_network_byte_order(0),
    m_valid(true) { }

tristan::network::Url::Url(const std::string& url) :
    m_port_local_byte_order(0),
    m_port_network_byte_order(0),
    m_valid(false) {
    auto l_url = url;
    auto scheme_end = l_url.find(':');
    if (scheme_end == std::string::npos) {
        m_error = tristan::network::makeError(tristan::network::UrlErrors::BAD_URL_FORMAT);
        return;
    }
    m_scheme = url.substr(0, scheme_end);
    auto port = tristan::network::schemes::getNetworkSchemeDefaultPort(m_scheme);
    if (port != 0){
        Url::setPort(port);
    }
    l_url.erase(0, scheme_end + 1);
    auto authority_start = l_url.find("//");
    if (authority_start != std::string::npos) {
        l_url.erase(0, 2);
        auto authority_end = l_url.find('/', authority_start);
        auto authority = l_url.substr(0, authority_end);
        l_url.erase(0, authority_end);
        auto user_info_end = authority.find_last_of('@');
        if (user_info_end != std::string::npos) {
            auto user_info = authority.substr(0, user_info_end);
            auto user_name_end = user_info.find(':');
            if (user_name_end == std::string::npos) {
                m_user_name = user_info;
            } else {
                m_user_name = user_info.substr(0, user_name_end);
                m_user_password = user_info.substr(user_name_end + 1);
            }
            authority.erase(0, user_info_end + 1);
        }
        auto port_start = authority.find(':');
        if (port_start != std::string::npos) {
            Url::setPort(authority.substr(port_start + 1));
            authority.erase(port_start);
        }
        std::regex regex(g_ip_regex_pattern);
        std::smatch ip_check_result;
        std::regex_match(authority, ip_check_result, regex);
        if (ip_check_result.empty()) {
            m_host = std::move(authority);
            Url::_resolveHost();
            if (m_error){
                return;
            }
        } else {
            IP ip;
            ip.as_int = tristan::network::utility::stringIpToUint32_tIp(authority);
            if (ip.as_int == 0){
                m_error = tristan::network::makeError(tristan::network::UrlErrors::IP_CONVERTER_ERROR);
                return;
            }
            ip.as_string = std::move(authority);
            m_host_ip.emplace_back(std::move(ip));
        }
    }
    auto query_start = l_url.find('?');
    if (query_start == std::string::npos) {
        m_path = l_url;
    } else {
        m_path = l_url.substr(0, query_start);
        l_url.erase(0, query_start + 1);
        auto fragment_start = l_url.find('#');
        if (fragment_start == std::string::npos) {
            m_query = l_url;
        } else {
            m_query = l_url.substr(0, fragment_start);
            m_fragment = l_url.substr(fragment_start + 1);
        }
    }
    m_valid = true;
}

void tristan::network::Url::setScheme(const std::string& scheme) {
    if ((scheme.at(0) < 'A' || scheme.at(0) > 'z') || (scheme.at(0) > 'Z' && scheme.at(0) < 'a')) {
        m_valid = false;
        return;
    }
    const auto not_allowed_char
        = scheme.find_first_not_of("+-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    if (not_allowed_char != std::string::npos) {
        m_valid = false;
        return;
    }
    m_scheme = scheme;
    auto port = tristan::network::schemes::getNetworkSchemeDefaultPort(m_scheme);
    if (port != 0){
        Url::setPort(port);
    }
}

void tristan::network::Url::setAuthority(const std::string& host,
                                         const std::string& user_name,
                                         const std::string& user_password) {
    if (host.size() > 253) {
        m_valid = true;
        return;
    }
    const auto not_allowed_char
        = host.find_first_not_of("-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    if (not_allowed_char != std::string::npos) {
        m_valid = false;
        return;
    }
    if (host.front() == '-' || host.back() == '-') {
        m_valid = false;
        return;
    }
    m_host = host;
    Url::_resolveHost();
    if (m_error){
        m_valid = false;
        return;
    }
    m_user_name = user_name;
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = user_name.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_user_name.replace(char_to_encode, 1, g_percentage_encoding.at(m_user_name.at(char_to_encode)));
        ++char_to_encode;
    }
    m_user_password = user_password;
    char_to_encode = 0;
    while (true) {
        char_to_encode = user_name.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_user_name.replace(char_to_encode, 1, g_percentage_encoding.at(m_user_name.at(char_to_encode)));
        ++char_to_encode;
    }
}

void tristan::network::Url::addHostIP(const std::string& ip) {
    IP l_ip;
    l_ip.as_int = tristan::network::utility::stringIpToUint32_tIp(ip);
    if (l_ip.as_int == 0){
        m_error = tristan::network::makeError(tristan::network::UrlErrors::IP_CONVERTER_ERROR);
        m_valid = false;
        return;
    }
    l_ip.as_string = ip;
    m_host_ip.emplace_back(l_ip);
}

void tristan::network::Url::setPort(uint16_t port) {
    m_port = std::to_string(port);
    m_port_local_byte_order = port;
    m_port_network_byte_order = htons(m_port_local_byte_order);
}

void tristan::network::Url::setPort(const std::string& port) {
    m_port = port;
    m_port_local_byte_order = static_cast< uint16_t >(std::stoi(m_port));
    m_port_network_byte_order = htons(m_port_local_byte_order);
}

void tristan::network::Url::setPath(const std::string& path) {
    m_path = path;
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = m_path.find_first_of(" !@#$%&*()+=[];:\',?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_path.replace(char_to_encode, 1, g_percentage_encoding.at(m_path.at(char_to_encode)));
        ++char_to_encode;
    }
}

void tristan::network::Url::setQuery(const std::string& query) {
    m_query = query;
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = m_query.find_first_of(" !@#$%*()+[]:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_query.replace(char_to_encode, 1, g_percentage_encoding.at(m_query.at(char_to_encode)));
        ++char_to_encode;
    }
}

void tristan::network::Url::setFragment(const std::string& fragment) {
    m_fragment = fragment;
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = m_fragment.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_fragment.replace(char_to_encode, 1, g_percentage_encoding.at(m_fragment.at(char_to_encode)));
        ++char_to_encode;
    }
}

auto tristan::network::Url::composeUrl() const -> std::string {
    std::string uri;
    if (not m_scheme.empty()){
        uri += m_scheme;
        uri += ':';
    }
    if (not m_host.empty() || not m_host_ip.empty()) {
        uri += "//";
        if (not m_user_name.empty()) {
            uri += m_user_name;
            if (not m_user_password.empty()) {
                uri += ':';
                uri += m_user_password;
            }
            uri += '@';
        }
        if (not m_host.empty()){
            uri += m_host;
        }
        else{
            uri += m_host_ip.at(0).as_string;
        }
        if (not m_port.empty()) {
            uri += ':';
            uri += m_port;
        }
    }
    uri += m_path;
    if (not m_query.empty()) {
        uri += '?';
        uri += m_query;
    }
    if (not m_fragment.empty()) {
        uri += '#';
        uri += m_fragment;
    }

    return uri;
}

void tristan::network::Url::_resolveHost() {
    auto* resolver_results = gethostbyname(m_host.c_str());
    if (resolver_results == nullptr) {
        switch (h_errno) {
            case HOST_NOT_FOUND: {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::NOT_FOUND_ERRNO);
                break;
            }
            case TRY_AGAIN: {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::TRY_AGAIN_ERRNO);
                break;
            }
            case NO_RECOVERY: {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::NO_RECOVERY_ERRNO);
                break;
            }
            case NO_DATA: {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::NO_DATA_ERRNO);
                break;
            }
        }
        return;
    }
    uint8_t index = 0;
    while (true){
        auto* address = resolver_results->h_addr_list[index];
        if (address == nullptr){
            break;
        }
        IP ip;
        memmove(&ip.as_int, address, 4);
        auto string_ip = tristan::network::utility::uint32_tIpToStringIp(ip.as_int);
        if (not string_ip.empty()){
            ip.as_string = std::move(string_ip);
            m_host_ip.emplace_back(std::move(ip));
        } else {
            m_error = tristan::network::makeError(tristan::network::UrlErrors::IP_CONVERTER_ERROR);
            return;
        }
        ++index;
    }
}
