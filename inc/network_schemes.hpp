#ifndef NETWORK_SCHEMES_HPP
#define NETWORK_SCHEMES_HPP

#include <string>
#include <utility>

struct NetworkScheme {
    std::string name;
    uint16_t port;
};

namespace tristan::network::schemes {

    [[nodiscard]] auto getNetworkSchemeDefaultPort(const std::string& scheme_name) noexcept -> uint16_t;
    [[nodiscard]] auto getNetworkSchemeName(uint16_t port) noexcept -> std::string;

}  // namespace tristan::network::schemes

#endif  //NETWORK_SCHEMES_HPP
