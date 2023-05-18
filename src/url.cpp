#include "url.hpp"
#include "network_utility.hpp"
#include "network_schemes.hpp"
#include "network_logger.hpp"

#include <netdb.h>

#include <unordered_map>
#include <regex>
#include <cstring>

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

tristan::network::Url::Url(const std::string& p_url) :
    m_port_local_byte_order(0),
    m_port_network_byte_order(0),
    m_valid(false) {
    netInfo("Parsing url from " + p_url);
    auto l_url = p_url;
    auto scheme_end = l_url.find(':');
    if (scheme_end == std::string::npos) {
        m_error = tristan::network::makeError(tristan::network::UrlErrors::BAD_URL_FORMAT);
        return;
    }
    m_scheme = p_url.substr(0, scheme_end);
    netDebug("m_scheme = " + m_scheme);
    auto port = tristan::network::schemes::getNetworkSchemeDefaultPort(m_scheme);
    if (port != 0) {
        Url::setPort(port);
        netDebug("m_port = " + std::to_string(m_port_local_byte_order));
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
            netDebug("m_user_name = " + m_user_name);
            netDebug("m_user_password = " + m_user_password);
            authority.erase(0, user_info_end + 1);
        }
        auto port_start = authority.find(':');
        if (port_start != std::string::npos) {
            Url::setPort(authority.substr(port_start + 1));
            authority.erase(port_start);
            netDebug("m_port = " + std::to_string(m_port_local_byte_order));
        }
        std::regex regex(g_ip_regex_pattern);
        std::smatch ip_check_result;
        std::regex_match(authority, ip_check_result, regex);
        if (ip_check_result.empty()) {
            m_host = std::move(authority);
            netDebug("m_host = " + m_host);
            Url::_resolveHost();
            if (m_error) {
                netError(m_error.message());
                return;
            }
        } else {
            IP ip;
            ip.as_int = tristan::network::utility::stringIpToUint32_tIp(authority);
            if (ip.as_int == 0) {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::IP_CONVERTER_ERROR);
                return;
            }
            ip.as_string = std::move(authority);
            netInfo("Host is set to " + ip.as_string);
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
    netDebug("m_path = " + m_path);
    netDebug("m_query = " + m_query);
    netDebug("m_fragment = " + m_fragment);
    m_valid = true;
}

void tristan::network::Url::setScheme(const std::string& p_scheme) {
    if ((p_scheme.at(0) < 'A' || p_scheme.at(0) > 'z') || (p_scheme.at(0) > 'Z' && p_scheme.at(0) < 'a')) {
        m_valid = false;
        return;
    }
    const auto not_allowed_char = p_scheme.find_first_not_of("+-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    if (not_allowed_char != std::string::npos) {
        m_valid = false;
        return;
    }
    m_scheme = p_scheme;
    netDebug("m_scheme = " + m_scheme);
    auto port = tristan::network::schemes::getNetworkSchemeDefaultPort(m_scheme);
    if (port != 0) {
        Url::setPort(port);
        netDebug("m_port = " + std::to_string(m_port_local_byte_order));
    }
}

void tristan::network::Url::setAuthority(const std::string& p_host, const std::string& p_user_name, const std::string& p_user_password) {
    if (p_host.size() > 253) {
        m_error = tristan::network::makeError(tristan::network::UrlErrors::BAD_HOST_SIZE);
        netError(m_error.message());
        m_valid = false;
        return;
    }
    const auto not_allowed_char = p_host.find_first_not_of("-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    if (not_allowed_char != std::string::npos) {
        m_error = tristan::network::makeError(tristan::network::UrlErrors::BAD_HOST_FORMAT);
        netError(m_error.message());
        m_valid = false;
        return;
    }
    if (p_host.front() == '-' || p_host.back() == '-') {
        m_valid = false;
        m_error = tristan::network::makeError(tristan::network::UrlErrors::BAD_HOST_FORMAT);
        netError(m_error.message());
        return;
    }
    m_host = p_host;
    netDebug("m_host = " + m_host);
    Url::_resolveHost();
    if (m_error) {
        netError(m_error.message());
        m_valid = false;
        return;
    }
    m_user_name = p_user_name;
    netDebug("m_user_name" + m_user_name);
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = p_user_name.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_user_name.replace(char_to_encode, 1, g_percentage_encoding.at(m_user_name.at(char_to_encode)));
        ++char_to_encode;
    }
    netDebug("m_user_name" + m_user_name);
    m_user_password = p_user_password;
    netDebug("m_user_password" + m_user_password);
    char_to_encode = 0;
    while (true) {
        char_to_encode = p_user_name.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_user_name.replace(char_to_encode, 1, g_percentage_encoding.at(m_user_name.at(char_to_encode)));
        ++char_to_encode;
    }
    netDebug("m_user_password" + m_user_password);
}

void tristan::network::Url::addHostIP(const std::string& p_ip) {
    netDebug("Adding host ip " + p_ip);
    std::regex regex(g_ip_regex_pattern);
    std::smatch ip_check_result;
    std::regex_match(p_ip, ip_check_result, regex);
    if (ip_check_result.empty()) {
        m_error = tristan::network::makeError(tristan::network::UrlErrors::BAD_IP_FORMAT);
        netError(m_error.message());
        m_valid = false;
        return;
    }
    IP l_ip;
    l_ip.as_int = tristan::network::utility::stringIpToUint32_tIp(p_ip);
    netDebug("l_ip.as_int = " + std::to_string(l_ip.as_int));
    if (l_ip.as_int == 0) {
        netError(m_error.message());
        m_error = tristan::network::makeError(tristan::network::UrlErrors::IP_CONVERTER_ERROR);
        m_valid = false;
        return;
    }
    l_ip.as_string = p_ip;
    m_host_ip.emplace_back(l_ip);
}

void tristan::network::Url::setPort(uint16_t p_port) {
    m_port = std::to_string(p_port);
    m_port_local_byte_order = p_port;
    m_port_network_byte_order = htons(m_port_local_byte_order);
    netDebug("m_port_network_byte_order = " + std::to_string(m_port_network_byte_order));
}

void tristan::network::Url::setPort(const std::string& p_port) {
    m_port = p_port;
    m_port_local_byte_order = static_cast< uint16_t >(std::stoi(m_port));
    m_port_network_byte_order = htons(m_port_local_byte_order);
    netDebug("m_port_network_byte_order = " + std::to_string(m_port_network_byte_order));
}

void tristan::network::Url::setPath(const std::string& p_path) {
    m_path = p_path;
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = m_path.find_first_of(" !@#$%&*()+=[];:\',?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_path.replace(char_to_encode, 1, g_percentage_encoding.at(m_path.at(char_to_encode)));
        ++char_to_encode;
    }
    netDebug("m_path = " + m_path);
}

void tristan::network::Url::setQuery(const std::string& p_query) {
    m_query = p_query;
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = m_query.find_first_of(" !@#$%*()+[]:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_query.replace(char_to_encode, 1, g_percentage_encoding.at(m_query.at(char_to_encode)));
        ++char_to_encode;
    }
    netDebug("m_query = " + m_query);
}

void tristan::network::Url::setFragment(const std::string& p_fragment) {
    m_fragment = p_fragment;
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = m_fragment.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        m_fragment.replace(char_to_encode, 1, g_percentage_encoding.at(m_fragment.at(char_to_encode)));
        ++char_to_encode;
    }
    netDebug("m_fragment = " + m_fragment);
}

auto tristan::network::Url::scheme() const noexcept -> const std::string& { return m_scheme; }

auto tristan::network::Url::userName() const noexcept -> const std::string& { return m_user_name; }

auto tristan::network::Url::userPassword() const noexcept -> const std::string& { return m_user_password; }

auto tristan::network::Url::host() const noexcept -> const std::string& { return m_host; }

auto tristan::network::Url::hostIP() const noexcept -> IP {
    if (not m_host.empty()) {
        return m_host_ip.at(0);
    }
    return {};
}

auto tristan::network::Url::hostIPList() const noexcept -> const std::vector< IP >& { return m_host_ip; }

auto tristan::network::Url::port() const noexcept -> const std::string& { return m_port; }

auto tristan::network::Url::portUint16_t_local_byte_order() const noexcept -> uint16_t { return static_cast< uint16_t >(m_port_local_byte_order); }

auto tristan::network::Url::portUint16_t_network_byte_order() const noexcept -> uint16_t { return static_cast< uint16_t >(m_port_network_byte_order); }

auto tristan::network::Url::path() const noexcept -> const std::string& { return m_path; }

auto tristan::network::Url::query() const noexcept -> const std::string& { return m_query; }

auto tristan::network::Url::fragment() const noexcept -> const std::string& { return m_fragment; }

auto tristan::network::Url::composeUrl() const -> std::string {
    netInfo("Composing url");
    std::string uri;
    if (not m_scheme.empty()) {
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
        if (not m_host.empty()) {
            uri += m_host;
        } else {
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
    netInfo("Composed url = " + uri);
    return uri;
}

auto tristan::network::Url::isValid() const noexcept -> bool { return m_valid; }

auto tristan::network::Url::error() const noexcept -> std::error_code { return m_error; }

void tristan::network::Url::_resolveHost() {
    netInfo("Resolving host " + m_host);
    auto* resolver_results = gethostbyname(m_host.c_str());
    if (resolver_results == nullptr) {
        switch (h_errno) {
            case HOST_NOT_FOUND: {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::NOT_FOUND_ERROR);
                break;
            }
            case TRY_AGAIN: {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::TRY_AGAIN_ERROR);
                break;
            }
            case NO_RECOVERY: {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::NO_RECOVERY_ERROR);
                break;
            }
            case NO_DATA: {
                m_error = tristan::network::makeError(tristan::network::UrlErrors::NO_DATA_ERROR);
                break;
            }
        }
        return;
    }
    uint8_t index = 0;
    while (true) {
        auto* address = resolver_results->h_addr_list[index];
        if (address == nullptr) {
            break;
        }
        IP ip;
        std::memmove(&ip.as_int, address, 4);
        auto string_ip = tristan::network::utility::uint32_tIpToStringIp(ip.as_int);
        if (not string_ip.empty()) {
            ip.as_string = std::move(string_ip);
            netInfo("Host was resolved to " + ip.as_string);
            m_host_ip.emplace_back(std::move(ip));
        } else {
            m_error = tristan::network::makeError(tristan::network::UrlErrors::IP_CONVERTER_ERROR);
            return;
        }
        ++index;
    }
}
