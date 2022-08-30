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

    public:
        NetworkResponse() = delete;

        /**
         * \brief Returns request uuid.
         * \return const std::string&
         */
        [[nodiscard]] auto uuid() const noexcept -> const std::string& { return m_uuid; }

        /**
         * \brief Provide access to data received from the remote.
         * \return const std::vector<uint8_t>&.
         */
        [[nodiscard]] auto data() const -> std::shared_ptr< std::vector< uint8_t > > { return m_response_data; }

        void setResponseData(std::shared_ptr< std::vector< uint8_t > > response_data);
    protected:
        explicit NetworkResponse(std::string uuid);

        NetworkResponse(const NetworkResponse& other) = default;
        NetworkResponse(NetworkResponse&& other) noexcept = default;

        NetworkResponse& operator=(const NetworkResponse& other) = default;
        NetworkResponse& operator=(NetworkResponse&& other) noexcept = default;

        ~NetworkResponse() = default;

        std::string m_uuid;

        std::shared_ptr< std::vector< uint8_t > > m_response_data;
    };

}  // namespace tristan::network

#endif  // NETWORK_RESPONSE_HPP
