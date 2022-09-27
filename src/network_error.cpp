#include "network_error.hpp"

#include <array>
#include <string>

namespace /*anonymous*/
{

    /**
   * \private
   * \brief Struct to overload std::error_category methods
   */
    struct NetworkCategory : std::error_category {
        [[nodiscard]] const char* name() const noexcept override;

        [[nodiscard]] std::string message(int ec) const override;
    };

    inline const NetworkCategory g_network_error_category;

    inline const uint8_t g_max_error_code = 8;

    inline const std::array< const char*, g_max_error_code > g_error_code_descriptions{
        "Success",
        "Network offline",
        "Invalid Url",
        "Remote host not found",
        "Output file path is empty",
        "Destination directory does not exists",
        "Downloader run() function invoked twice",
        "Request has not either bytes to read either delimiter"};

    /**
   * \private
   * \brief Struct to overload std::error_category methods
   */
    struct UrlErrorCategory : std::error_category {
        [[nodiscard]] const char* name() const noexcept override;

        [[nodiscard]] std::string message(int ec) const override;
    };

    inline const UrlErrorCategory g_url_error_category;

    inline const uint8_t g_max_url_code = 7;

    inline const std::array< const char*, g_max_url_code > g_url_code_descriptions{
        "Success",
        "Host not found",
        "A temporary error occurred on an authoritative name server. Try again later",
        "A nonrecoverable name server error occurred",
        "The requested name is valid but does not have an IP address. Another type of request to the "
        "name server for this domain may return an answer",
        "Bad url format",
        "IP address conversion failed"
    };

}  // namespace

auto tristan::network::makeError(tristan::network::ErrorCode error_code) -> std::error_code {
    return {static_cast< int >(error_code), g_network_error_category};
}

auto tristan::network::makeError(tristan::network::UrlErrors error_code) -> std::error_code {
    return {static_cast< int >(error_code), g_url_error_category};
}

namespace /*anonymous*/
{
    auto NetworkCategory::name() const noexcept -> const char* { return "NetworkCategory"; }

    auto NetworkCategory::message(int ec) const -> std::string { return {g_error_code_descriptions.at(ec)}; }

    auto UrlErrorCategory::name() const noexcept -> const char* { return "UrlCategory"; }

    auto UrlErrorCategory::message(int ec) const -> std::string { return {g_url_code_descriptions.at(ec)}; }
}  // namespace
