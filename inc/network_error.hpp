#ifndef NETWORK_ERROR_HPP
#define NETWORK_ERROR_HPP

#include <system_error>
#include <cstdint>


namespace tristan::network {

    enum class ErrorCode : uint8_t {
        SUCCESS,
        OFFLINE,
        INVALID_URL,
        HOST_NOT_FOUND,
        FILE_PATH_EMPTY,
        DESTINATION_DIR_DOES_NOT_EXISTS,
        ASYNC_NETWORK_REQUEST_HANDLER_LUNCHED_TWICE,
        ASYNC_NETWORK_REQUEST_HANDLER_WAS_NOT_LUNCHED,
        REQUEST_SIZE_IS_NOT_APPROPRIATE,
        REQUEST_NOT_SUPPORTED
    };

    enum class UrlErrors : uint8_t {
        SUCCESS,
        NOT_FOUND_ERROR,
        TRY_AGAIN_ERROR,
        NO_RECOVERY_ERROR,
        NO_DATA_ERROR,
        BAD_HOST_SIZE,
        BAD_HOST_FORMAT,
        BAD_URL_FORMAT,
        BAD_IP_FORMAT,
        IP_CONVERTER_ERROR,
    };

    enum class NetworkResponseError : uint8_t {
        SUCCESS,
        HTTP_BAD_RESPONSE_FORMAT,
        HTTP_RESPONSE_SIZE_ERROR,
    };

    /**
     * \brief Creates std::error_code object that stores error information.
     * \param error_code ErrorCode
     * \return std::error_code
     */
    [[nodiscard]] auto makeError(ErrorCode error_code) -> std::error_code;

    /**
     * \brief Creates std::error_code object that stores error information.
     * \param error_code UrlErrors
     * \return std::error_code
     */
    [[nodiscard]] auto makeError(UrlErrors error_code) -> std::error_code;

    /**
     * \brief Creates std::error_code object that stores error information.
     * \param error_code SocketErrors
     * \return std::error_code
     */
    [[nodiscard]] auto makeError(NetworkResponseError error_code) -> std::error_code;

}  // namespace tristan::network

namespace std {

    /**
     * \brief //This is needed to specialise the standard type trait.
     */
    template <>
    struct is_error_code_enum< tristan::network::ErrorCode > : true_type {
    };

    /**
     * \brief //This is needed to specialise the standard type trait.
     */
    template <>
    struct is_error_code_enum< tristan::network::UrlErrors > : true_type {
    };

    /**
     * \brief //This is needed to specialise the standard type trait.
     */
    template <>
    struct is_error_code_enum< tristan::network::NetworkResponseError > : true_type {
    };

}  // namespace std

#endif  // NETWORK_ERROR_HPP
