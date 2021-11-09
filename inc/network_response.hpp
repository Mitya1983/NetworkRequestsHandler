#ifndef NETWORK_RESPONSE_HPP
#define NETWORK_RESPONSE_HPP

#include <string>
#include <utility>

namespace tristan::network{
    /**
     * \class NetworkResponse.
     * \brief Used a a base class for network responses.
     */
    class NetworkResponse{
      public:
        [[nodiscard]] auto uuid() const noexcept -> const std::string&{ return m_uuid; }

      protected:
        explicit NetworkResponse(std::string uuid) :
                m_uuid(std::move(uuid)){}

        NetworkResponse(const NetworkResponse& other) = default;
        NetworkResponse(NetworkResponse&& other) = default;

        NetworkResponse& operator=(const NetworkResponse& other) = default;
        NetworkResponse& operator=(NetworkResponse&& other) = default;

        ~NetworkResponse() = default;

      private:
        std::string m_uuid;
    };

} //End of tristan::network namespace

#endif //NETWORK_RESPONSE_HPP
