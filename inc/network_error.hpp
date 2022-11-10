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
        REQUEST_SIZE_IS_NOT_APPROPRIATE
    };

    enum class UrlErrors : uint8_t {
        SUCCESS,
        NOT_FOUND_ERROR,
        TRY_AGAIN_ERROR,
        NO_RECOVERY_ERROR,
        NO_DATA_ERROR,
        BAD_URL_FORMAT,
        BAD_IP_FORMAT,
        IP_CONVERTER_ERROR,
    };
    enum class SocketErrors : uint8_t {
        SUCCESS,
        SOCKET_PROTOCOL_NOT_SUPPORTED,
        SOCKET_PROCESS_TABLE_IS_FULL,
        SOCKET_SYSTEM_TABLE_IS_FULL,
        SOCKET_NOT_ENOUGH_PERMISSIONS,
        SOCKET_NOT_ENOUGH_MEMORY,
        SOCKET_WRONG_PROTOCOL,
        SOCKET_WRONG_IP_FORMAT,
        SOCKET_NOT_INITIALISED,
        SOCKET_NOT_CONNECTED,
        CONNECT_NOT_ENOUGH_PERMISSIONS,
        CONNECT_ADDRESS_IN_USE,
        CONNECT_ADDRESS_NOT_AVAILABLE,
        CONNECT_AF_NOT_SUPPORTED,
        CONNECT_TRY_AGAIN,
        CONNECT_ALREADY_IN_PROCESS,
        CONNECT_BAD_FILE_DESCRIPTOR,
        CONNECT_CONNECTION_REFUSED,
        CONNECT_ADDRESS_OUTSIDE_USER_SPACE,
        CONNECT_IN_PROGRESS,
        CONNECT_INTERRUPTED,
        CONNECT_CONNECTED,
        CONNECT_NETWORK_UNREACHABLE,
        CONNECT_FILE_DESCRIPTOR_IS_NOT_SOCKET,
        CONNECT_PROTOCOL_NOT_SUPPORTED,
        CONNECT_TIMED_OUT,
        WRITE_TRY_AGAIN,
        WRITE_BAD_FILE_DESCRIPTOR,
        WRITE_DESTINATION_ADDRESS,
        WRITE_USER_QUOTA,
        WRITE_BUFFER_OUT_OF_RANGE,
        WRITE_BIG,
        WRITE_INTERRUPTED,
        WRITE_INVALID_ARGUMENT,
        WRITE_LOW_LEVEL_IO,
        WRITE_NO_SPACE,
        WRITE_NOT_PERMITTED,
        WRITE_PIPE,
        WRITE_TIMED_OUT,
        READ_TRY_AGAIN,
        READ_BAD_FILE_DESCRIPTOR,
        READ_BUFFER_OUT_OF_RANGE,
        READ_INTERRUPTED,
        READ_INVALID_FILE_DESCRIPTOR,
        READ_IO,
        READ_IS_DIRECTORY,
        READ_TIMED_OUT
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
    [[nodiscard]] auto makeError(SocketErrors error_code) -> std::error_code;

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
    struct is_error_code_enum< tristan::network::SocketErrors > : true_type {
    };

}  // namespace std

#endif  // NETWORK_ERROR_HPP
