#include "network_error.hpp"

#include <map>
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

    inline const std::map< tristan::network::ErrorCode, const char* > g_error_code_descriptions{
        {tristan::network::ErrorCode::SUCCESS,                                       "Success"                                              },
        {tristan::network::ErrorCode::OFFLINE,                                       "Network offline"                                      },
        {tristan::network::ErrorCode::INVALID_URL,                                   "Invalid Url"                                          },
        {tristan::network::ErrorCode::HOST_NOT_FOUND,                                "Remote host not found"                                },
        {tristan::network::ErrorCode::FILE_PATH_EMPTY,                               "Output file path is empty"                            },
        {tristan::network::ErrorCode::DESTINATION_DIR_DOES_NOT_EXISTS,               "Destination directory does not exists"                },
        {tristan::network::ErrorCode::ASYNC_NETWORK_REQUEST_HANDLER_LUNCHED_TWICE,   "AsyncRequestHandler run() function invoked twice"     },
        {tristan::network::ErrorCode::ASYNC_NETWORK_REQUEST_HANDLER_WAS_NOT_LUNCHED, "AsyncRequestHandler run() function was not invoked"   },
        {tristan::network::ErrorCode::REQUEST_SIZE_IS_NOT_APPROPRIATE,               "Request has not either bytes to read either delimiter"},
        {tristan::network::ErrorCode::REQUEST_NOT_SUPPORTED,                         "Request type is not supported"                        },
    };

    /**
   * \private
   * \brief Struct to overload std::error_category methods
   */
    struct UrlErrorCategory : std::error_category {
        [[nodiscard]] const char* name() const noexcept override;

        [[nodiscard]] std::string message(int ec) const override;
    };

    inline const UrlErrorCategory g_url_error_category;

    inline const std::map< tristan::network::UrlErrors, const char* > g_url_code_descriptions{
        {tristan::network::UrlErrors::SUCCESS,            "Success"                                                                                    },
        {tristan::network::UrlErrors::NOT_FOUND_ERROR,    "Host not found"                                                                             },
        {tristan::network::UrlErrors::TRY_AGAIN_ERROR,    "A temporary error occurred on an authoritative name server. Try again later"                },
        {tristan::network::UrlErrors::NO_RECOVERY_ERROR,  "A nonrecoverable name server error occurred"                                                },
        {tristan::network::UrlErrors::NO_DATA_ERROR,
         "The requested name is valid but does not have an IP address. Another type of request to the name server for this domain may return an answer"},
        {tristan::network::UrlErrors::BAD_HOST_SIZE,      "Host size is to big"                                                                        },
        {tristan::network::UrlErrors::BAD_HOST_FORMAT,    "Host contains not allowed characters"                                                       },
        {tristan::network::UrlErrors::BAD_URL_FORMAT,     "Bad url format"                                                                             },
        {tristan::network::UrlErrors::BAD_IP_FORMAT,      "Bad IP format"                                                                              },
        {tristan::network::UrlErrors::IP_CONVERTER_ERROR, "IP address conversion failed"                                                               },
    };

    struct NetworkResponseCategory : std::error_category {
        [[nodiscard]] const char* name() const noexcept override;

        [[nodiscard]] std::string message(int ec) const override;
    };

    inline const NetworkResponseCategory g_network_response_error_category;

    inline const std::map< tristan::network::NetworkResponseError, const char* > g_network_response_code_descriptions{
        {tristan::network::NetworkResponseError::SUCCESS,                  "Success"                                                                         },
        {tristan::network::NetworkResponseError::HTTP_BAD_RESPONSE_FORMAT, "Bad format of the received http response"                                        },
        {tristan::network::NetworkResponseError::HTTP_RESPONSE_SIZE_ERROR, "Content-length and transfer-encoding chunked are not present in response headers"},
    };

}  // namespace

auto tristan::network::makeError(tristan::network::ErrorCode p_error_code) -> std::error_code {
    return {static_cast< int >(p_error_code), g_network_error_category};
}

auto tristan::network::makeError(tristan::network::UrlErrors p_error_code) -> std::error_code { return {static_cast< int >(p_error_code), g_url_error_category}; }

auto tristan::network::makeError(tristan::network::NetworkResponseError p_error_code) -> std::error_code {
    return {static_cast< int >(p_error_code), g_network_response_error_category};
    ;
}

namespace /*anonymous*/
{
    auto NetworkCategory::name() const noexcept -> const char* { return "NetworkCategory"; }

    auto NetworkCategory::message(int ec) const -> std::string { return {g_error_code_descriptions.at(static_cast< tristan::network::ErrorCode >(ec))}; }

    auto UrlErrorCategory::name() const noexcept -> const char* { return "UrlCategory"; }

    auto UrlErrorCategory::message(int ec) const -> std::string { return {g_url_code_descriptions.at(static_cast< tristan::network::UrlErrors >(ec))}; }

    auto NetworkResponseCategory::name() const noexcept -> const char* { return "NetworkResponseCategory"; }

    auto NetworkResponseCategory::message(int ec) const -> std::string {
        return {g_network_response_code_descriptions.at(static_cast< tristan::network::NetworkResponseError >(ec))};
    }
}  // namespace
