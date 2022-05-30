#ifndef NETWORK_ERROR_HPP
#define NETWORK_ERROR_HPP

#include <system_error>
#include <cstdint>

namespace tristan::network{

    enum class ErrorCode : uint8_t{
        SUCCESS,
        OFFLINE,
        INVALID_URL,
        HOST_NOT_FOUND,
        FILE_PATH_EMPTY,
        DESTINATION_DIR_DOES_NOT_EXISTS
    };

    /**
     * \brief Creates std::error_code object that stores error information.
     * \param error_code ErrorCode
     * \return std::error_code
     */
    auto makeError(ErrorCode error_code) -> std::error_code;

} //End of tristan::network namespace

namespace std{

    /**
   * \brief //This is needed to specialise the standard type trait.
   */
    template<>
    struct is_error_code_enum<tristan::network::ErrorCode> : true_type{
    };

} //End of std namespace

#endif //NETWORK_ERROR_HPP
