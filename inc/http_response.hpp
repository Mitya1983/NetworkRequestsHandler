#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "status_codes.hpp"
#include "network_response.hpp"

#include <string>
#include <unordered_map>
#include <filesystem>
#include <optional>

namespace tristan::network{

    class HttpResponse : public NetworkResponse{
        friend class HttpRequest;

        explicit HttpResponse(const std::string& response_base, std::string uuid);
      public:
        HttpResponse(const HttpResponse& other) = default;
        HttpResponse(HttpResponse&& other) = default;
        HttpResponse& operator=(const HttpResponse& other) = default;
        HttpResponse& operator=(HttpResponse&& other) = default;
        ~HttpResponse() = default;

        auto headerValue(const std::string& header_name) const -> std::optional<std::string>;

        auto data() const -> const std::string&{ return m_data; }

        auto status() const -> HttpStatus{ return m_status; }

        auto statusDescription() const -> const std::string&;

      private:
        std::string m_data;
        std::unordered_map<std::string, std::string> m_headers;
        HttpStatus m_status;
    };

} //End of tristan::network namespace

#endif //HTTP_RESPONSE_HPP
