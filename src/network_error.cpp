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

    inline const uint8_t g_max_error_code = 8;

    inline const std::array< const char*, g_max_error_code > g_error_code_descriptions{"Success",
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
        "IP address conversion failed"};

    struct SocketErrorCategory : std::error_category {
        [[nodiscard]] const char* name() const noexcept override;

        [[nodiscard]] std::string message(int ec) const override;
    };

    inline const SocketErrorCategory g_socket_error_category;

    inline const std::map< tristan::network::SocketErrors, const char* > g_socket_code_descriptions{
        {tristan::network::SocketErrors::SUCCESS,                               "Success"                                                                               },
        {tristan::network::SocketErrors::SOCKET_PROTOCOL_NOT_SUPPORTED,
         "The protocol type or the specified protocol is not supported within this communication domain"                                                                },
        {tristan::network::SocketErrors::SOCKET_PROCESS_TABLE_IS_FULL,          "The per-process descriptor table is full"                                              },
        {tristan::network::SocketErrors::SOCKET_SYSTEM_TABLE_IS_FULL,           "The system file table is full"                                                         },
        {tristan::network::SocketErrors::SOCKET_NOT_ENOUGH_PERMISSIONS,         "Permission to create a socket of the specified type and/or protocol is denied"         },
        {tristan::network::SocketErrors::SOCKET_NOT_ENOUGH_MEMORY,
         "Insufficient buffer space is available. The socket cannot be created until sufficient resources are freed"                                                    },
        {tristan::network::SocketErrors::SOCKET_WRONG_PROTOCOL,                 "The protocol is the wrong type for the socket"                                         },
        {tristan::network::SocketErrors::SOCKET_WRONG_IP_FORMAT,                "Wrong ip format"                                                                       },
        {tristan::network::SocketErrors::SOCKET_NOT_INITIALISED,                "Socket is not initialised"                                                             },
        {tristan::network::SocketErrors::SOCKET_NOT_CONNECTED,                  "Socket is not connected"                                                             },
        {tristan::network::SocketErrors::CONNECT_NOT_ENOUGH_PERMISSIONS,        "Local address is already in use"                                                       },
        {tristan::network::SocketErrors::CONNECT_ADDRESS_IN_USE,
         "For UNIX domain sockets, which are identified by pathname: Write permission is denied on the socket file, or search permission "
         "is denied for one of the directories in the path prefix. The user tried to connect to a broadcast address without having the "
         "socket broadcast flag enabled or the connection request failed because of a local firewall rule"                                                              },
        {tristan::network::SocketErrors::CONNECT_ADDRESS_NOT_AVAILABLE,
         "The socket referred to had not previously been bound to an address and, upon attempting to bind it to an ephemeral port"                                      },
        {tristan::network::SocketErrors::CONNECT_AF_NOT_SUPPORTED,              "The passed address didn't have the correct address family in its sa_family field"      },
        {tristan::network::SocketErrors::CONNECT_TRY_AGAIN,
         "For nonblocking UNIX domain sockets, the socket is nonblocking, and the connection cannot be completed immediately.  For other "
         "socket families, there are insufficient entries in the routing cache"                                                                                         },
        {tristan::network::SocketErrors::CONNECT_ALREADY_IN_PROCESS,            "The socket is nonblocking and a previous connection attempt has not yet been completed"},
        {tristan::network::SocketErrors::CONNECT_BAD_FILE_DESCRIPTOR,           "Socket is not a valid open file descriptor"                                            },
        {tristan::network::SocketErrors::CONNECT_CONNECTION_REFUSED,            "On a stream socket found no one listening on the remote address"                       },
        {tristan::network::SocketErrors::CONNECT_ADDRESS_OUTSIDE_USER_SPACE,    "The socket structure address is outside the user's address space"                      },
        {tristan::network::SocketErrors::CONNECT_IN_PROGRESS,                   "The socket is nonblocking and the connection cannot be completed immediately"          },
        {tristan::network::SocketErrors::CONNECT_INTERRUPTED,                   "The system call was interrupted by a signal that was caught"                           },
        {tristan::network::SocketErrors::CONNECT_CONNECTED,                     "The socket is already connected"                                                       },
        {tristan::network::SocketErrors::CONNECT_NETWORK_UNREACHABLE,           "Network is unreachable"                                                                },
        {tristan::network::SocketErrors::CONNECT_FILE_DESCRIPTOR_IS_NOT_SOCKET, "The file descriptor does not refer to a socket"                                        },
        {tristan::network::SocketErrors::CONNECT_PROTOCOL_NOT_SUPPORTED,
         "The protocol type or the specified protocol is not supported within this communication domain"                                                                },
        {tristan::network::SocketErrors::CONNECT_TIMED_OUT,                     "Timeout while attempting connection"                                                   },
        {tristan::network::SocketErrors::WRITE_TRY_AGAIN,                       "The socket is marked nonblocking and the requested operation would block"              },
        {tristan::network::SocketErrors::WRITE_BAD_FILE_DESCRIPTOR,             "Socket is not a valid open file descriptor"                                            },
        {tristan::network::SocketErrors::WRITE_DESTINATION_ADDRESS,
         "The socket refers to a datagram socket for which a peer address has not been set using connect"                                                               },
        {tristan::network::SocketErrors::WRITE_USER_QUOTA,
         "he user's quota of disk blocks on the filesystem containing the file referred to by fd has been exhausted"                                                    },
        {tristan::network::SocketErrors::WRITE_INTERRUPTED,                     "A signal occurred before any data was transmitted"                                     },
        {tristan::network::SocketErrors::WRITE_BUFFER_OUT_OF_RANGE,             "Buffer is outside your accessible address space"                                       },
        {tristan::network::SocketErrors::WRITE_BIG,
         "An attempt was made to write a file that exceeds the implementation-defined maximum file size or the process's file size limit, or to write at a "
         "position past the maximum allowed offset"                                                                                                                     },
        {tristan::network::SocketErrors::WRITE_INVALID_ARGUMENT,                "Invalid argument passed"                                                               },
        {tristan::network::SocketErrors::WRITE_LOW_LEVEL_IO,                    "A low-level I/O error occurred while modifying the inode"                              },
        {tristan::network::SocketErrors::WRITE_NO_SPACE,                        "The device containing the file referred to by fd has no room for the data"             },
        {tristan::network::SocketErrors::WRITE_NOT_PERMITTED,                   "The operation was prevented by a file seal"                                            },
        {tristan::network::SocketErrors::WRITE_PIPE,                            "The socket is connected to a pipe or socket whose reading end is closed"               },
    };

}  // namespace

auto tristan::network::makeError(tristan::network::ErrorCode error_code) -> std::error_code {
    return {static_cast< int >(error_code), g_network_error_category};
}

auto tristan::network::makeError(tristan::network::UrlErrors error_code) -> std::error_code { return {static_cast< int >(error_code), g_url_error_category}; }

auto tristan::network::makeError(tristan::network::SocketErrors error_code) -> std::error_code {
    return {static_cast< int >(error_code), g_socket_error_category};
}

namespace /*anonymous*/
{
    auto NetworkCategory::name() const noexcept -> const char* { return "NetworkCategory"; }

    auto NetworkCategory::message(int ec) const -> std::string { return {g_error_code_descriptions.at(ec)}; }

    auto UrlErrorCategory::name() const noexcept -> const char* { return "UrlCategory"; }

    auto UrlErrorCategory::message(int ec) const -> std::string { return {g_url_code_descriptions.at(ec)}; }

    auto SocketErrorCategory::name() const noexcept -> const char* { return "SocketCategory"; }

    auto SocketErrorCategory::message(int ec) const -> std::string {
        return {g_socket_code_descriptions.at(static_cast< tristan::network::SocketErrors >(ec))};
    }
}  // namespace
