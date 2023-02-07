#include "http_param.hpp"

#include <algorithm>

tristan::network::HttpParams::HttpParams(const std::string& params_data) {
    size_t parameter_name_start = 0;
    while (true) {
        size_t parameter_name_end = params_data.find('=');
        if (parameter_name_end == std::string::npos) {
            break;
        }
        auto parameter_name
            = params_data.substr(parameter_name_start, parameter_name_end - parameter_name_start);
        size_t parameter_value_start = parameter_name_end + 1;
        size_t parameter_value_end = params_data.find('&', parameter_value_start);
        auto parameter_value
            = params_data.substr(parameter_value_start, parameter_value_end - parameter_value_start);
        m_params.emplace_back(std::move(parameter_name), std::move(parameter_value));
    }
}

tristan::network::HttpParams::HttpParams(const std::vector< uint8_t >& params_data) :
    tristan::network::HttpParams(std::string(params_data.begin(), params_data.end())) { }

void tristan::network::HttpParams::addParameter(tristan::network::Parameter&& parameter) {
    m_params.emplace_back(std::move(parameter));
}

auto tristan::network::HttpParams::parameterValue(const std::string& parameter_name) const
    -> std::optional< std::string > {
    auto found = std::find_if(m_params.begin(),
                              m_params.end(),
                              [parameter_name](const tristan::network::Parameter& header) -> bool {
                                  return header.name == parameter_name;
                              });
    if (found == m_params.cend()) {
        return std::nullopt;
    }
    return found->value;
}

auto tristan::network::HttpParams::empty() -> bool { return m_params.empty(); }
