#include "socket.hpp"
#include "network_error.hpp"
#include "net_log.hpp"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>

//TODO: Add EOF check in socket::read functions

tristan::network::Socket::Socket(tristan::network::SocketType socket_type) :
    m_socket(-1),
    m_ip(0),
    m_port(0),
    m_type(socket_type),
    m_non_blocking(false),
    m_connected(false),
    m_ssl_connected(false) {

    netInfo("Creating socket of type " + std::to_string(static_cast< int >(socket_type)));
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
        netError(m_error.message());
    }
}

tristan::network::Socket::~Socket() { ::close(m_socket); }

void tristan::network::Socket::setHost(uint32_t ip, const std::string& host_name) {
    m_ip = ip;
    if (not host_name.empty()) {
        m_host_name = host_name;
    }
}

void tristan::network::Socket::setRemotePort(uint16_t port) { m_port = port; }

void tristan::network::Socket::setNonBlocking(bool non_blocking) {
    netInfo("Setting socket as nonblocking");
    if (m_socket == -1) {
        m_error = tristan::network::makeError(tristan::network::SocketErrors::SOCKET_NOT_INITIALISED);
        netError(m_error.message());
        return;
    }
    int32_t status;
    if (non_blocking) {
        status = fcntl(m_socket, F_SETFL, O_NONBLOCK);
    } else {
        status = fcntl(m_socket, F_SETFL, 0);
    }
    if (status < 0) {
        m_error = tristan::network::makeError(tristan::network::SocketErrors::SOCKET_FCNTL_ERROR);
        netError(m_error.message());
        netError("Fcntl error code is " + std::to_string(errno));
        return;
    }
    m_non_blocking = non_blocking;
}

void tristan::network::Socket::resetError() { m_error = tristan::network::makeError(tristan::network::SocketErrors::SUCCESS); }

void tristan::network::Socket::connect(bool ssl) {
    if (not m_connected) {
        netInfo("Connecting to " + (m_host_name.empty() ? std::to_string(m_ip) : m_host_name) + ":" + std::to_string(m_port));
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
            netError(m_error.message());
            return;
        }
        m_connected = true;
    }
    if (ssl && not m_ssl_connected) {
        try {
            m_ssl = tristan::network::Ssl::create(m_socket);
        } catch (const std::system_error& error) {
            m_error = error.code();
            netError(error.what());
            return;
        }

        m_error = m_ssl->connect();

        if (m_error.value() == static_cast< int >(tristan::network::SocketErrors::SSL_TRY_AGAIN)) {
            m_error = tristan::network::makeError(tristan::network::SocketErrors::CONNECT_TRY_AGAIN);
        }

        if (m_error) {
            return;
        }

        bool certificate_verified;
        if (not m_host_name.empty()) {
            certificate_verified = m_ssl->verifyHost(m_host_name);
        } else {
            certificate_verified = m_ssl->verifyIp(m_ip);
        }
        bool start_date_is_valid = m_ssl->verifyStartDate();
        bool end_date_is_valid = m_ssl->verifyEndDate();
        if (not certificate_verified && not start_date_is_valid && not end_date_is_valid) {
            m_error = tristan::network::makeError(tristan::network::SocketErrors::SSL_CERTIFICATE_VERIFICATION_HOST);
            netDebug("Certificate verified = " + (certificate_verified ? std::string("true") : std::string("false")));
            netDebug("Certificate start date is valid = " + (certificate_verified ? std::string("true") : std::string("false")));
            netDebug("Certificate end date is valid = " + (certificate_verified ? std::string("true") : std::string("false")));
            netError(m_error.message());
            return;
        }
        m_ssl_connected = true;
    }
    netInfo("Connection to " + (m_host_name.empty() ? std::to_string(m_ip) : m_host_name) + " was successful");
}

void tristan::network::Socket::close() {
    if (m_ssl) {
        if (m_error.value() != static_cast< int >(tristan::network::SocketErrors::SSL_IO_ERROR)
            && m_error.value() != static_cast< int >(tristan::network::SocketErrors::SSL_FATAL_ERROR)) {
            m_ssl->shutdown();
        }
        m_ssl.reset();
    }
    ::close(m_socket);
}

auto tristan::network::Socket::write(const std::vector< uint8_t >& data, uint16_t size, uint64_t offset) -> uint64_t {
    netInfo("Writing to " + (m_host_name.empty() ? std::to_string(m_ip) : m_host_name) + ":" + std::to_string(m_port));

    if (data.empty()) {
        netError("Data is empty");
        return 0;
    }

    if (m_ssl && not m_ssl_connected) {
        m_error = tristan::network::makeError(tristan::network::SocketErrors::SOCKET_NOT_CONNECTED);
        return 0;
    }
    netDebug("data = " + std::string(data.begin(), data.end()));
    netDebug("size = " + std::to_string(size));
    netDebug("offset = " + std::to_string(offset));
    uint64_t bytes_sent = 0;
    uint64_t l_size = size;
    if (l_size == 0) {
        l_size = data.size();
    }

    if (m_connected) {
        if (m_ssl) {
            netDebug("SSL_write_ex(m_ssl->ssl, data.data() + offset, static_cast<uint64_t>(size), &bytes_sent);");
            auto ssl_write_result = m_ssl->write(data, l_size, offset);
            bytes_sent = ssl_write_result.second;
            if (ssl_write_result.first && ssl_write_result.first.value() == static_cast< int >(tristan::network::SocketErrors::SSL_TRY_AGAIN)) {
                m_error = tristan::network::makeError(tristan::network::SocketErrors::WRITE_TRY_AGAIN);
            } else {
                m_error = ssl_write_result.first;
            }
            return bytes_sent;
        }
        netDebug("::write(m_socket, data.data() + offset, l_size);");
        bytes_sent = ::write(m_socket, data.data() + offset, l_size);
    } else {
        if (m_type == tristan::network::SocketType::TCP) {
            m_error = tristan::network::makeError(tristan::network::SocketErrors::SOCKET_NOT_CONNECTED);
            netError(m_error.message());
        } else {
            netDebug(
                ":sendto(m_socket, data.data() + offset, l_size, MSG_NOSIGNAL, reinterpret_cast< struct sockaddr* >(&remote_address), sizeof(remote_address))");
            sockaddr_in remote_address{};
            remote_address.sin_family = AF_INET;
            remote_address.sin_addr.s_addr = m_ip;
            remote_address.sin_port = m_port;
            bytes_sent
                = ::sendto(m_socket, data.data() + offset, l_size, MSG_NOSIGNAL, reinterpret_cast< struct sockaddr* >(&remote_address), sizeof(remote_address));
        }
    }
    if (static_cast< int64_t >(bytes_sent) < 0) {
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
        netError(m_error.message());
    }
    return bytes_sent;
}

auto tristan::network::Socket::read() -> uint8_t {

    uint8_t byte = 0;

    if (m_ssl) {
        auto ssl_read_status = m_ssl->read();
        byte = ssl_read_status.second;
        if (ssl_read_status.first && ssl_read_status.first.value() == static_cast< int >(tristan::network::SocketErrors::SSL_TRY_AGAIN)) {
            m_error = tristan::network::makeError(tristan::network::SocketErrors::WRITE_TRY_AGAIN);
        } else {
            m_error = ssl_read_status.first;
        }
        return byte;
    }

    auto status = ::read(m_socket, &byte, 1);
    if (status < 0) {
        tristan::network::SocketErrors error = tristan::network::SocketErrors::SUCCESS;
        switch (errno) {
            case EAGAIN: {
                error = tristan::network::SocketErrors::READ_TRY_AGAIN;
                break;
            }
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
            case EWOULDBLOCK: {
                error = tristan::network::SocketErrors::READ_TRY_AGAIN;
                break;
            }
#endif
            case EBADF: {
                error = tristan::network::SocketErrors::READ_BAD_FILE_DESCRIPTOR;
                break;
            }
            case EFAULT: {
                error = tristan::network::SocketErrors::READ_BUFFER_OUT_OF_RANGE;
                break;
            }
            case EINTR: {
                error = tristan::network::SocketErrors::READ_INTERRUPTED;
                break;
            }
            case EINVAL: {
                error = tristan::network::SocketErrors::READ_INVALID_FILE_DESCRIPTOR;
                break;
            }
            case EIO: {
                error = tristan::network::SocketErrors::READ_IO;
                break;
            }
            case EISDIR: {
                error = tristan::network::SocketErrors::READ_IS_DIRECTORY;
                break;
            }
        }
        m_error = tristan::network::makeError(error);
        netError(m_error.message());
    }
    if (byte == 255){
        m_error = tristan::network::makeError(tristan::network::SocketErrors::READ_EOF);
        byte = 0;
    }
    return byte;
}

auto tristan::network::Socket::read(uint16_t size) -> std::vector< uint8_t > {

    netInfo("Reading from " + (m_host_name.empty() ? std::to_string(m_ip) : m_host_name) + ":" + std::to_string(m_port));
    if (size == 0) {
        netError("Size is 0");
        return {};
    }

    std::vector< uint8_t > data;

    if (m_ssl) {
        auto ssl_read_status = m_ssl->read(data, size);
        if (ssl_read_status.first && ssl_read_status.first.value() == static_cast< int >(tristan::network::SocketErrors::SSL_TRY_AGAIN)) {
            m_error = tristan::network::makeError(tristan::network::SocketErrors::WRITE_TRY_AGAIN);
        } else {
            m_error = ssl_read_status.first;
        }
        return data;
    }

    data.resize(size);
    netDebug("size = " + std::to_string(size));
    auto status = ::read(m_socket, data.data(), size);
    if (status < 0) {
        tristan::network::SocketErrors error = tristan::network::SocketErrors::SUCCESS;
        switch (errno) {
            case EAGAIN: {
                error = tristan::network::SocketErrors::READ_TRY_AGAIN;
                break;
            }
            case EBADF: {
                error = tristan::network::SocketErrors::READ_BAD_FILE_DESCRIPTOR;
                break;
            }
            case EFAULT: {
                error = tristan::network::SocketErrors::READ_BUFFER_OUT_OF_RANGE;
                break;
            }
            case EINTR: {
                error = tristan::network::SocketErrors::READ_INTERRUPTED;
                break;
            }
            case EINVAL: {
                error = tristan::network::SocketErrors::READ_INVALID_FILE_DESCRIPTOR;
                break;
            }
            case EIO: {
                error = tristan::network::SocketErrors::READ_IO;
                break;
            }
            case EISDIR: {
                error = tristan::network::SocketErrors::READ_IS_DIRECTORY;
                break;
            }
        }
        m_error = tristan::network::makeError(error);
        netError(m_error.message());
    }

    //    for (uint16_t i = 0; i < size; ++i) {
    //        uint8_t byte = Socket::read();
    //        if (m_error) {
    //            break;
    //        }
    //        data.push_back(byte);
    //    }
    if (data.at(0) == 0) {
        return {};
    }
    return data;
}

auto tristan::network::Socket::readUntil(uint8_t delimiter) -> std::vector< uint8_t > {

    netInfo("Reading from " + (m_host_name.empty() ? std::to_string(m_ip) : m_host_name) + ":" + std::to_string(m_port) + " until "
            + std::to_string(delimiter));
    std::vector< uint8_t > data;

    while (true) {
        uint8_t byte = Socket::read();
        if (m_error || byte == delimiter || byte == 0) {
            break;
        }
        data.push_back(byte);
    }
    netDebug("data = " + std::string(data.begin(), data.end()));
    return data;
}

auto tristan::network::Socket::readUntil(const std::vector< uint8_t >& delimiter) -> std::vector< uint8_t > {
    netInfo("Reading from " + (m_host_name.empty() ? std::to_string(m_ip) : m_host_name) + ":" + std::to_string(m_port) + " until "
            + std::string(delimiter.begin(), delimiter.end()));

    std::vector< uint8_t > data;
    data.reserve(delimiter.size());

    while (true) {
        uint8_t byte = Socket::read();
        if (m_error || byte == 0) {
            break;
        }
        data.push_back(byte);
        if (data.size() >= delimiter.size()) {
            std::vector< uint8_t > to_compare(data.end() - static_cast< int64_t >(delimiter.size()), data.end());
            if (to_compare == delimiter) {
                break;
            }
        } else if (data.size() == delimiter.size() && data == delimiter) {
            break;
        }
    }
    netDebug("data = " + std::string(data.begin(), data.end()));
    return data;
}

auto tristan::network::Socket::ip() const noexcept -> uint32_t { return m_ip; }

auto tristan::network::Socket::port() const noexcept -> uint16_t { return m_port; }

auto tristan::network::Socket::error() const noexcept -> std::error_code { return m_error; }

auto tristan::network::Socket::nonBlocking() const noexcept -> bool { return m_non_blocking; }

auto tristan::network::Socket::connected() const noexcept -> bool { return m_connected; }
