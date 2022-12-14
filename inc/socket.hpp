#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "ssl.hpp"

#include <cstdint>
#include <string>
#include <system_error>
#include <memory>
#include <vector>

namespace tristan::network {

    enum class SocketType : uint8_t {
        TCP,
        UDP
    };

    class Socket {
    public:
        explicit Socket(SocketType socket_type = SocketType::TCP);
        Socket(const Socket& other) = delete;
        Socket(Socket&& other) = default;

        Socket& operator=(const Socket& other) = delete;
        Socket& operator=(Socket&& other) = default;

        ~Socket();

        void setHost(uint32_t ip, const std::string& host_name = "");
        void setRemotePort(uint16_t port);
        void setNonBlocking(bool non_blocking = true);
        void resetError();
        void connect(bool ssl = true);
        void close();
        [[nodiscard]] auto write(const std::vector< uint8_t >& data, uint16_t size = 0, uint64_t offset = 0) -> uint64_t;

        template < class ObjectClassToSend >
        [[nodiscard]] auto write(ObjectClassToSend object) -> uint64_t
            requires std::is_standard_layout_v< ObjectClassToSend >
        {
            std::vector< uint8_t > temp_data(reinterpret_cast< uint8_t* >(object), reinterpret_cast< uint8_t* >(object) + sizeof(object));
            return Socket::write(temp_data);
        }

        [[nodiscard]] auto read() -> uint8_t;
        [[nodiscard]] auto read(uint16_t size) -> std::vector< uint8_t >;
        [[nodiscard]] auto readUntil(uint8_t delimiter) -> std::vector< uint8_t >;
        [[nodiscard]] auto readUntil(const std::vector< uint8_t >& delimiter) -> std::vector< uint8_t >;

        [[nodiscard]] auto ip() const noexcept -> uint32_t;
        [[nodiscard]] auto port() const noexcept -> uint16_t;
        [[nodiscard]] auto error() const noexcept -> std::error_code;
        [[nodiscard]] auto nonBlocking() const noexcept -> bool;
        [[nodiscard]] auto connected() const noexcept -> bool;

    protected:
    private:

        std::string m_host_name;

        int32_t m_socket;
        uint32_t m_ip;

        std::error_code m_error;

        uint16_t m_port;

        std::unique_ptr<Ssl> m_ssl;
        SocketType m_type;

        bool m_non_blocking;
        bool m_not_ssl_connected;
        bool m_connected;
    };

}  // namespace tristan::network

#endif  //SOCKET_HPP
