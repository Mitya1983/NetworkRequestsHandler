#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "status_codes.hpp"
#include "network_response.hpp"

#include <string>
#include <unordered_map>
#include <filesystem>
#include <optional>
#include <vector>

namespace tristan::network{
    /**
     * \class HttpResponse
     * \brief Handles HTTP response. This class is created in \class HttpRequest
     * \extends  NetworkResponse
     */
    class HttpResponse : public NetworkResponse<std::string>{
        friend class HttpRequest;
        /**
         * \brief Constructor
         * \param response_base const std::string&. Base received from remote
         * \param uuid std::string. UUID is passed from \class NetworkRequest
         */
        explicit HttpResponse(const std::string& response_base, std::string uuid);
      public:
        HttpResponse(const HttpResponse& other) = default;
        HttpResponse(HttpResponse&& other) noexcept = default;
        HttpResponse& operator=(const HttpResponse& other) = default;
        HttpResponse& operator=(HttpResponse&& other) noexcept = default;
        ~HttpResponse() = default;
        /**
         * \brief Return value for specified header.
         * \param header_name const std::string&
         * \return Header value if header was set by the remote, empty string otherwise.
         */
        auto headerValue(const std::string& header_name) const -> std::optional<std::string>;

        /**
         * \brief Returns http status code
         * \return HttpStatus
         */
        auto status() const -> HttpStatus{ return m_status; }

        /**
         * \brief Returns http status description in [code]: [Description] format.
         * \return const std::string&
         */
        auto statusDescription() const -> const std::string&;

      private:
        std::unordered_map<std::string, std::string> m_headers;
        HttpStatus m_status;
    };

} //End of tristan::network namespace

#endif //HTTP_RESPONSE_HPP
