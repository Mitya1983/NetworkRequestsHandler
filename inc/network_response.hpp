#ifndef NETWORK_RESPONSE_HPP
#define NETWORK_RESPONSE_HPP

#include <string>
#include <vector>
#include <memory>

namespace tristan::network {
    /**
     * \class NetworkResponse.
     * \brief Used as a base class for network responses.
     */
    class NetworkResponse {
    friend class NetworkRequestBase;
    public:
        NetworkResponse() = delete;

        NetworkResponse(const NetworkResponse& other) = delete;
        NetworkResponse(NetworkResponse&& other) noexcept = default;
        NetworkResponse& operator=(const NetworkResponse& other) = delete;
        NetworkResponse& operator=(NetworkResponse&& other) noexcept = default;

        virtual ~NetworkResponse() = default;

        [[nodiscard]] static auto createResponse(std::string uuid) -> std::shared_ptr<NetworkResponse>;

        /**
         * \brief Returns request uuid.
         * \return const std::string&
         */
        [[nodiscard]] auto uuid() const noexcept -> const std::string&;
        /**
         * \brief Provide access to data received from the remote.
         * \return const std::vector<uint8_t>&.
         */
        [[nodiscard]] auto data() const -> std::shared_ptr< std::vector< uint8_t > >;

    protected:
        explicit NetworkResponse(std::string&& uuid);

        std::string m_uuid;

        std::shared_ptr< std::vector< uint8_t > > m_response_data;
    };

}  // namespace tristan::network

#endif  // NETWORK_RESPONSE_HPP
