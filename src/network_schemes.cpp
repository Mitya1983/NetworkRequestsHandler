#include "network_schemes.hpp"
#include <array>

namespace {

    inline const std::array< NetworkScheme, 7 > g_network_schemes = {
        NetworkScheme{"ftp",   20 },
        NetworkScheme{"ssh",   22 },
        NetworkScheme{"smtp",  25 },
        NetworkScheme{"http",  80 },
        NetworkScheme{"imap",  143},
        NetworkScheme{"https", 443},
        NetworkScheme{"pop",   995}
    };
}  //End of anonymous namespace

auto tristan::network::schemes::getNetworkSchemeDefaultPort(const std::string& scheme_name) noexcept
    -> uint16_t {
    for (const auto& scheme: g_network_schemes) {
        if (scheme.name == scheme_name) {
            return scheme.port;
        }
    }
    return 0;
}

auto tristan::network::schemes::getNetworkSchemeName(uint16_t port) noexcept -> std::string {
    for (const auto& scheme: g_network_schemes) {
        if (scheme.port == port) {
            return scheme.name;
        }
    }
    return {};
}
