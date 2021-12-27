#ifndef NETWORK_RESPONSE_HPP
#define NETWORK_RESPONSE_HPP

#include <string>
#include <utility>
#include <vector>
#include <concepts>
#include <cstdint>

namespace tristan::network{
    /**
     * \class NetworkResponse.
     * \brief Used as a base class for network responses.
     */

    template<class Data> concept BinaryArrayType = std::is_same_v<std::string, Data> || std::is_same_v<std::vector<uint8_t>, Data>;

    template<BinaryArrayType Data> class NetworkResponse{

      public:
        /**
         * \brief Returns request uuid.
         * \return Data or empty string if data was written into file or remote didn't provided any (e.g. in case of failure or redirection).
         */
        [[nodiscard]] auto uuid() const noexcept -> const std::string&{ return m_uuid; }

        /**
         * \brief Provide access to data received from the remote.
         * \return Data or empty string if data was written into file or remote didn't provided any (e.g. in case of failure or redirection).
         */
        [[nodiscard]] auto data() const -> const Data&{ return m_data; }

      protected:
        explicit NetworkResponse(std::string uuid) :
                m_uuid(std::move(uuid)){}

        NetworkResponse(const NetworkResponse& other) = default;
        NetworkResponse(NetworkResponse&& other) noexcept = default;

        NetworkResponse& operator=(const NetworkResponse& other) = default;
        NetworkResponse& operator=(NetworkResponse&& other)  noexcept = default;

        ~NetworkResponse() = default;

        std::string m_uuid;

        Data m_data;
    };

    template<class Response> concept IsDerivedFromNetworkResponse = std::is_base_of_v<NetworkResponse<std::string>, Response> || std::is_base_of_v<NetworkResponse<std::vector<uint8_t>>, Response>;

} //End of tristan::network namespace

#endif //NETWORK_RESPONSE_HPP
