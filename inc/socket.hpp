#ifndef NETWORKREQUESTHANDLER_SOCKET_HPP
#define NETWORKREQUESTHANDLER_SOCKET_HPP


#include <cstdint>
#include <string>
#include <system_error>
#include <vector>

namespace tristan::network {

    enum class SocketType : uint8_t {
        TCP,
        UDP
    };

    class Socket
    {
    public:
        explicit Socket(SocketType socket_type = SocketType::TCP);
        Socket(const Socket& other) = delete;
        Socket(Socket&& other) = default;

        Socket& operator=(const Socket& other) = delete;
        Socket& operator=(Socket&& other) = default;

        ~Socket();

        void setRemoteIp(const std::string& ip);
        void setRemotePort(uint16_t port);
        void setNonBlocking(bool non_blocking = true);
        void connect();
        auto write(const std::vector<uint8_t>& data) -> uint64_t;
        template < class ObjectClassToSend > auto write(ObjectClassToSend object) -> uint64_t requires std::is_standard_layout_v<ObjectClassToSend>{
            std::vector<uint8_t> temp_data(reinterpret_cast<uint8_t*>(object), reinterpret_cast<uint8_t*>(object) + sizeof (object));
            return Socket::write(temp_data);
        }
        [[nodiscard]] auto read() const -> uint8_t;
        [[nodiscard]] auto read(uint16_t size) const -> std::vector<uint8_t>;
        [[nodiscard]] auto readUntil(uint8_t delimiter) const -> std::vector<uint8_t>;
        [[nodiscard]] auto readUntil(const std::vector<uint8_t>& delimiter) const -> std::vector<uint8_t>;

        protected:

    private:
        int32_t m_socket;
        uint32_t m_ip;

        std::error_code m_error;

        uint16_t m_port;
        SocketType m_type;

        bool m_connected;
    };

} //End of tristan::network namespace

#endif  //NETWORKREQUESTHANDLER_SOCKET_HPP
