#include "url.hpp"

#include <unordered_map>

namespace{

    inline const std::unordered_map<char, std::string> percentage_encoding = { { ' ',  "%20" },
                                                                               { '!',  "%21" },
                                                                               { '@',  "%40" },
                                                                               { '#',  "%23" },
                                                                               { '$',  "%24" },
                                                                               { '%',  "%25" },
                                                                               { '&',  "%26" },
                                                                               { '*',  "%2A" },
                                                                               { '(',  "%28" },
                                                                               { ')',  "%29" },
                                                                               { '+',  "%2B" },
                                                                               { '=',  "%3D" },
                                                                               { '[',  "%5B" },
                                                                               { ']',  "%5D" },
                                                                               { ':',  "%3A" },
                                                                               { ';',  "%3B" },
                                                                               { '\'', "%27" },
                                                                               { ',',  "%2C" },
                                                                               { '/',  "%2F" },
                                                                               { '?',  "%3F" },
    };

} //End of unnamed namespace

tristan::network::Url::Url() noexcept:
        m_valid(true){

}

tristan::network::Url::Url(const std::string& url) :
        m_valid(false){
    auto l_url = url;
    auto scheme_end = l_url.find(':');
    if (scheme_end == std::string::npos){
        return;
    }
    m_scheme = url.substr(0, scheme_end);
    l_url.erase(0, scheme_end + 1);
    auto authority_start = l_url.find("//");
    if (authority_start != std::string::npos){
        l_url.erase(0, 2);
        auto authority_end = l_url.find('/', authority_start);
        auto authority = l_url.substr(0, authority_end);
        l_url.erase(0, authority_end);
        auto user_info_end = authority.find_last_of('@');
        if (user_info_end != std::string::npos){
            auto user_info = authority.substr(0, user_info_end);
            auto user_name_end = user_info.find(':');
            if (user_name_end == std::string::npos){
                m_user_name = user_info;
            }
            else{
                m_user_name = user_info.substr(0, user_name_end);
                m_user_password = user_info.substr(user_name_end + 1);
            }
            authority.erase(0, user_info_end + 1);
        }
        auto port_start = authority.find(':');
        if (port_start != std::string::npos){
            m_port = authority.substr(port_start + 1);
            authority.erase(port_start);
        }
        m_host = std::move(authority);
    }
    auto query_start = l_url.find('?');
    if (query_start == std::string::npos){
        m_path = l_url;
    }
    else{
        m_path = l_url.substr(0, query_start);
        l_url.erase(0, query_start + 1);
        auto fragment_start = l_url.find('#');
        if (fragment_start == std::string::npos){
            m_query = l_url;
        }
        else{
            m_query = l_url.substr(0, fragment_start);
            m_fragment = l_url.substr(fragment_start + 1);
        }
    }
    m_valid = true;
}

void tristan::network::Url::setScheme(const std::string& scheme){
    if ((scheme.at(0) < 'A' || scheme.at(0) > 'z') || (scheme.at(0) > 'Z' && scheme.at(0) < 'a')){
        m_valid = false;
        return;
    }
    m_scheme = scheme;
    const auto not_allowed_char = scheme.find_first_not_of("+-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    if (not_allowed_char != std::string::npos){
        m_valid = false;
        return;
    }
    m_scheme = scheme;
}

void tristan::network::Url::setAuthority(const std::string& host, const std::string& user_name, const std::string& user_password){
    if (host.size() > 253){
        m_valid = true;
        return;
    }
    const auto not_allowed_char = host.find_first_not_of("-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    if (not_allowed_char != std::string::npos){
        m_valid = false;
        return;
    }
    if (host.front() == '-' || host.back() == '-'){
        m_valid = false;
        return;
    }
    m_host = host;
    m_user_name = user_name;
    size_t char_to_encode = 0;
    while (true){
        char_to_encode = user_name.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos){
            break;
        }
        m_user_name.replace(char_to_encode, 1, percentage_encoding.at(m_user_name.at(char_to_encode)));
        ++char_to_encode;
    }
    m_user_password = user_password;
    char_to_encode = 0;
    while (true){
        char_to_encode = user_name.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos){
            break;
        }
        m_user_name.replace(char_to_encode, 1, percentage_encoding.at(m_user_name.at(char_to_encode)));
        ++char_to_encode;
    }
}

void tristan::network::Url::setHostIP(const std::string& ip) {
    m_host_ip = ip;
}

void tristan::network::Url::setPort(uint16_t port){
    m_port = std::to_string(port);
}

void tristan::network::Url::setPort(const std::string& port){
    m_port = port;
}

void tristan::network::Url::setPath(const std::string& path){
    m_path = path;
    size_t char_to_encode = 0;
    while (true){
        char_to_encode = m_path.find_first_of(" !@#$%&*()+=[];:\',?", char_to_encode);
        if (char_to_encode == std::string::npos){
            break;
        }
        m_path.replace(char_to_encode, 1, percentage_encoding.at(m_path.at(char_to_encode)));
        ++char_to_encode;
    }
}

void tristan::network::Url::setQuery(const std::string& query){
    m_query = query;
    size_t char_to_encode = 0;
    while (true){
        char_to_encode = m_query.find_first_of(" !@#$%*()+[]:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos){
            break;
        }
        m_query.replace(char_to_encode, 1, percentage_encoding.at(m_query.at(char_to_encode)));
        ++char_to_encode;
    }
}

void tristan::network::Url::setFragment(const std::string& fragment){
    m_fragment = fragment;
    size_t char_to_encode = 0;
    while (true){
        char_to_encode = m_fragment.find_first_of(" !@#$%&*()+=[];:\',/?", char_to_encode);
        if (char_to_encode == std::string::npos){
            break;
        }
        m_fragment.replace(char_to_encode, 1, percentage_encoding.at(m_fragment.at(char_to_encode)));
        ++char_to_encode;
    }
}

auto tristan::network::Url::composeUrl(bool ip_address) const -> std::string{
    auto uri = m_scheme;
    uri += ':';
    if (!m_host.empty()){
        uri += "//";
        if (!m_user_name.empty()){
            uri += m_user_name;
            if (!m_user_password.empty()){
                uri += ':';
                uri += m_user_password;
            }
            uri += '@';
        }
        uri += m_host;
        if (!m_port.empty()){
            uri += ':';
            uri += m_port;
        }
    }
    uri += m_path;
    if (!m_query.empty()){
        uri += '?';
        uri += m_query;
    }
    if (!m_fragment.empty()){
        uri += '#';
        uri += m_fragment;
    }

    return uri;
}
