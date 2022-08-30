#include "network_error.hpp"

#include <array>
#include <string>

namespace /*anonymous*/
{

    /**
   * \private
   * \brief Struct to overload std::error_category methods
   */
    struct NetworkCategory : std::error_category{
        [[nodiscard]] const char* name() const noexcept override;

        [[nodiscard]] std::string message(int ec) const override;
    };

    inline const NetworkCategory g_network_error_category;

    inline const uint8_t g_max_error_code = 7;

    inline const std::array<const char*, g_max_error_code> g_error_code_descriptions{
        "Success",
        "Network offline",
        "Invalid Url",
        "Remote host not found",
        "Output file path is empty",
        "Destination directory does not exists",
        "Downloader run() function invoked twice",
    };

} //End of /*anonymous*/ namespace

auto tristan::network::makeError(tristan::network::ErrorCode error_code) -> std::error_code{
    return { static_cast<int>(error_code),
             g_network_error_category
    };
}

namespace /*anonymous*/
{
    auto NetworkCategory::name() const noexcept -> const char*{
        return "TristanNetworkCategory";
    }

    auto NetworkCategory::message(int ec) const -> std::string{
        return {g_error_code_descriptions.at(ec)};
    }
} //End of /*anonymous*/ namespace
