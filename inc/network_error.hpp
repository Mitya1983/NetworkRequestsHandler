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
        DOWNLOADER_LUNCHED_TWICE,
        REQUEST_SIZE_IS_NOT_APPROPRIATE
    };

    enum class UrlErrors : uint8_t {
        SUCCESS,
        NOT_FOUND_ERRNO,
        TRY_AGAIN_ERRNO,
        NO_RECOVERY_ERRNO,
        NO_DATA_ERRNO,
        BAD_URL_FORMAT,
        IP_CONVERTER_ERROR,
    };

    /**
     * \brief Creates std::error_code object that stores error information.
     * \param error_code ErrorCode
     * \return std::error_code
     */
    auto makeError(ErrorCode error_code) -> std::error_code;

    /**
     * \brief Creates std::error_code object that stores error information.
     * \param error_code ErrorCode
     * \return std::error_code
     */
    auto makeError(UrlErrors error_code) -> std::error_code;

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

}  // namespace std

#endif  // NETWORK_ERROR_HPP
