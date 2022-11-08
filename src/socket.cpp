#include "socket.hpp"
#include <network_utility.hpp>

#include <network_error.hpp>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>

#include <regex>

tristan::network::Socket::Socket(tristan::network::SocketType socket_type) :
    m_socket(-1),
    m_ip(0),
    m_port(0),
    m_type(socket_type),
    m_connected(false) {

    auto protocol = getprotobyname("ip");
    if (m_type == tristan::network::SocketType::TCP) {
        m_socket = socket(AF_INET, SOCK_STREAM, protocol->p_proto);
    } else {
        m_socket = socket(AF_INET, SOCK_DGRAM, protocol->p_proto);
    }
    if (m_socket < 0) {
        tristan::network::SocketErrors error;
        switch (errno) {
            case EPROTONOSUPPORT: {
                error = tristan::network::SocketErrors::SOCKET_PROTOCOL_NOT_SUPPORTED;
                break;
            }
            case EMFILE: {
                error = tristan::network::SocketErrors::SOCKET_PROCESS_TABLE_IS_FULL;
                break;
            }
            case ENFILE: {
                error = tristan::network::SocketErrors::SOCKET_SYSTEM_TABLE_IS_FULL;
                break;
            }
            case EACCES: {
                error = tristan::network::SocketErrors::SOCKET_NOT_ENOUGH_PERMISSIONS;
                break;
            }
            case ENOSR: {
                error = tristan::network::SocketErrors::SOCKET_NOT_ENOUGH_MEMORY;
                break;
            }
            case EPROTOTYPE: {
                error = tristan::network::SocketErrors::SOCKET_WRONG_PROTOCOL;
                break;
            }
        }
        m_error = tristan::network::makeError(error);
    }
}

tristan::network::Socket::~Socket() {
    close(m_socket);
}

void tristan::network::Socket::setRemoteIp(const std::string& ip) {
    std::regex regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
    std::smatch match_result;

    std::regex_match(ip, match_result, regex);
    if (match_result.empty()) {
        m_error = tristan::network::makeError(tristan::network::SocketErrors::SOCKET_WRONG_IP_FORMAT);
        return;
    }
    m_ip = tristan::network::utility::stringIpToUint32_tIp(ip);
}

void tristan::network::Socket::setRemotePort(uint16_t port) { m_port = htons(port); }

void tristan::network::Socket::setNonBlocking(bool non_blocking) {
    if (m_socket == -1) {
        m_error = tristan::network::makeError(tristan::network::SocketErrors::SOCKET_NOT_INITIALISED);
        return;
    }
    if (non_blocking) {
        fcntl(m_socket, F_SETFL, O_NONBLOCK);
    } else {
        fcntl(m_socket, F_SETFL, 0);
    }
}

void tristan::network::Socket::connect() {
    sockaddr_in remote_address{};
    remote_address.sin_family = AF_INET;
    remote_address.sin_addr.s_addr = m_ip;
    remote_address.sin_port = m_port;

    int32_t status = ::connect(m_socket, reinterpret_cast< struct sockaddr* >(&remote_address), sizeof(remote_address));
    if (status < 0) {
        tristan::network::SocketErrors error = tristan::network::SocketErrors::SUCCESS;
        switch (errno) {
            case EACCES: {
                error = tristan::network::SocketErrors::SOCKET_NOT_ENOUGH_PERMISSIONS;
                [[fallthrough]];
            }
            case EPERM: {
                break;
            }
            case EADDRINUSE: {
                error = tristan::network::SocketErrors::CONNECT_ADDRESS_IN_USE;
                break;
            }
            case EADDRNOTAVAIL: {
                error = tristan::network::SocketErrors::CONNECT_ADDRESS_NOT_AVAILABLE;
                break;
            }
            case EAFNOSUPPORT: {
                error = tristan::network::SocketErrors::CONNECT_AF_NOT_SUPPORTED;
                break;
            }
            case EAGAIN: {
                error = tristan::network::SocketErrors::CONNECT_TRY_AGAIN;
                break;
            }
            case EALREADY: {
                error = tristan::network::SocketErrors::CONNECT_ALREADY_IN_PROCESS;
                break;
            }
            case EBADF: {
                error = tristan::network::SocketErrors::CONNECT_BAD_FILE_DESCRIPTOR;
                break;
            }
            case ECONNREFUSED: {
                error = tristan::network::SocketErrors::CONNECT_CONNECTION_REFUSED;
                break;
            }
            case EFAULT: {
                error = tristan::network::SocketErrors::CONNECT_ADDRESS_OUTSIDE_USER_SPACE;
                break;
            }
            case EINPROGRESS: {
                error = tristan::network::SocketErrors::CONNECT_IN_PROGRESS;
                break;
            }
            case EINTR: {
                error = tristan::network::SocketErrors::CONNECT_INTERRUPTED;
                break;
            }
            case EISCONN: {
                error = tristan::network::SocketErrors::CONNECT_CONNECTED;
                break;
            }
            case ENETUNREACH: {
                error = tristan::network::SocketErrors::CONNECT_NETWORK_UNREACHABLE;
                break;
            }
            case ENOTSOCK: {
                error = tristan::network::SocketErrors::CONNECT_FILE_DESCRIPTOR_IS_NOT_SOCKET;
                break;
            }
            case EPROTOTYPE: {
                error = tristan::network::SocketErrors::CONNECT_PROTOCOL_NOT_SUPPORTED;
                break;
            }
            case ETIMEDOUT: {
                error = tristan::network::SocketErrors::CONNECT_TIMED_OUT;
                break;
            }
        }
        m_error = tristan::network::makeError(error);
    }
    m_connected = true;
}

auto tristan::network::Socket::write(const std::vector< uint8_t >& data) -> uint64_t {

    int64_t bytes_sent = 0;
    if (m_connected) {
        bytes_sent = ::write(m_socket, data.data(), data.size());
    } else {
        if (m_type == tristan::network::SocketType::TCP) {
            m_error = tristan::network::makeError(tristan::network::SocketErrors::SOCKET_NOT_CONNECTED);
        } else {
            sockaddr_in remote_address{};
            remote_address.sin_family = AF_INET;
            remote_address.sin_addr.s_addr = m_ip;
            remote_address.sin_port = m_port;
            bytes_sent
                = ::sendto(m_socket, data.data(), data.size(), MSG_NOSIGNAL, reinterpret_cast< struct sockaddr* >(&remote_address), sizeof(remote_address));
        }
    }
    if (bytes_sent < 0) {
        tristan::network::SocketErrors error = tristan::network::SocketErrors::SUCCESS;
        switch (errno) {
            case EAGAIN: {
                error = tristan::network::SocketErrors::WRITE_TRY_AGAIN;
                break;
            }
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
            case EWOULDBLOCK: {
                error = tristan::network::SocketErrors::WRITE_TRY_AGAIN;
                break;
            }
#endif
            case EBADF: {
                error = tristan::network::SocketErrors::WRITE_BAD_FILE_DESCRIPTOR;
                break;
            }
            case EDESTADDRREQ: {
                error = tristan::network::SocketErrors::WRITE_DESTINATION_ADDRESS;
                break;
            }
            case EFAULT: {
                error = tristan::network::SocketErrors::WRITE_BUFFER_OUT_OF_RANGE;
                break;
            }
            case EFBIG: {
                error = tristan::network::SocketErrors::WRITE_BIG;
                break;
            }
            case EINTR: {
                error = tristan::network::SocketErrors::WRITE_INTERRUPTED;
                break;
            }
            case EINVAL: {
                error = tristan::network::SocketErrors::WRITE_INVALID_ARGUMENT;
                break;
            }
            case EIO: {
                error = tristan::network::SocketErrors::WRITE_LOW_LEVEL_IO;
                break;
            }
            case ENOSPC: {
                error = tristan::network::SocketErrors::WRITE_NO_SPACE;
                break;
            }
            case EPERM: {
                error = tristan::network::SocketErrors::WRITE_NOT_PERMITTED;
                break;
            }
            case EPIPE: {
                error = tristan::network::SocketErrors::WRITE_PIPE;
                break;
            }
        }
        m_error = tristan::network::makeError(error);
    }
    return bytes_sent;
}

auto tristan::network::Socket::read() const -> uint8_t {
    uint8_t byte;
    ::read(m_socket, &byte, 1);
    return byte;
}

auto tristan::network::Socket::read(uint16_t size) const -> std::vector< uint8_t > {
    if (size == 0) {
        return {};
    }
    std::vector< uint8_t > data;
    data.reserve(size);

    for (uint16_t i = 0; i < size; ++i) {
        uint8_t byte = Socket::read();
        if (m_error) {
            break;
        }
        data.push_back(byte);
    }
    return data;
}

auto tristan::network::Socket::readUntil(uint8_t delimiter) const -> std::vector< uint8_t > {
    std::vector< uint8_t > data;

    while (true) {
        uint8_t byte = Socket::read();
        if (m_error || byte == delimiter) {
            break;
        }
        data.push_back(byte);
    }

    return data;
}

auto tristan::network::Socket::readUntil(const std::vector< uint8_t >& delimiter) const -> std::vector< uint8_t > {
    std::vector< uint8_t > data;
    data.reserve(delimiter.size());

    while (true){
        uint8_t byte = Socket::read();
        if (m_error) {
            break;
        }
        data.push_back(byte);
        if (data.size() > delimiter.size()){
            std::vector<uint8_t> to_compare(data.end() - static_cast<int64_t>(delimiter.size()), data.end());
            if (to_compare == delimiter){
                break;
            }
        } else if (data.size() == delimiter.size() && data == delimiter){
            break;
        }
        data.push_back(byte);
    }
    return data;
}
