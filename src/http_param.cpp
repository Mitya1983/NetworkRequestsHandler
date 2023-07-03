#include "http_param.hpp"

#include <algorithm>

tristan::network::HttpParams::HttpParams(const std::string& p_params_data) {
    size_t parameter_name_start = 0;
    while (true) {
        size_t parameter_name_end = p_params_data.find('=');
        if (parameter_name_end == std::string::npos) {
            break;
        }
        auto parameter_name
            = p_params_data.substr(parameter_name_start, parameter_name_end - parameter_name_start);
        size_t parameter_value_start = parameter_name_end + 1;
        size_t parameter_value_end = p_params_data.find('&', parameter_value_start);
        auto parameter_value
            = p_params_data.substr(parameter_value_start, parameter_value_end - parameter_value_start);
        m_params.emplace_back(std::move(parameter_name), std::move(parameter_value));
    }
}

tristan::network::HttpParams::HttpParams(const std::vector< uint8_t >& p_params_data) :
    tristan::network::HttpParams(std::string(p_params_data.begin(), p_params_data.end())) { }

void tristan::network::HttpParams::addParameter(tristan::network::Parameter&& p_parameter) {
    m_params.emplace_back(std::move(p_parameter));
}

auto tristan::network::HttpParams::parameterValue(const std::string& p_parameter_name) const
    -> std::optional< std::string > {
    auto found = std::find_if(m_params.begin(),
                              m_params.end(),
                              [p_parameter_name](const tristan::network::Parameter& header) -> bool {
                                  return header.m_name == p_parameter_name;
                              });
    if (found == m_params.cend()) {
        return std::nullopt;
    }
    return found->m_string;
}

auto tristan::network::HttpParams::empty() -> bool { return m_params.empty(); }

auto tristan::network::HttpParams::begin() noexcept -> std::vector< tristan::network::Parameter>::iterator {
    return m_params.begin();
}

auto tristan::network::HttpParams::cbegin() const noexcept -> std::vector< tristan::network::Parameter>::const_iterator {
    return m_params.cbegin();
}

auto tristan::network::HttpParams::end() noexcept -> std::vector< tristan::network::Parameter>::iterator {
    return m_params.end();
}

auto tristan::network::HttpParams::cend() const noexcept -> std::vector< tristan::network::Parameter>::const_iterator {
    return m_params.cend();
}
