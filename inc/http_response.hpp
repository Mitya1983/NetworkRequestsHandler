#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "network_response.hpp"
#include "http_header.hpp"
#include "http_param.hpp"
#include "http_status_codes.hpp"

namespace tristan::network {

    class HttpResponse : public NetworkResponse
    {
        friend class HttpRequest;
    public:
        HttpResponse() = delete;
        HttpResponse(const HttpResponse& p_other) = delete;
        HttpResponse(HttpResponse&& p_other) = default;

        HttpResponse& operator=(const HttpResponse& p_other) = delete;
        HttpResponse& operator=(HttpResponse&& p_other) = default;

        ~HttpResponse() override = default;

        [[nodiscard]] static auto createResponse(std::string p_uuid, std::vector<uint8_t>&& p_headers_data) -> std::shared_ptr<HttpResponse>;

        [[nodiscard]] auto error() const -> std::error_code;

        [[nodiscard]] auto status() const -> HttpStatus;

        [[nodiscard]] auto headers() const -> const std::unique_ptr<HttpHeaders>&;

    protected:
    private:
        explicit HttpResponse(std::string&& p_string, std::vector<uint8_t>&& p_headers_data);

        std::error_code m_error;

        std::unique_ptr<HttpHeaders> m_response_headers;

        HttpStatus m_status;
    };

} //End of tristan::network namespace

#endif  //HTTP_RESPONSE_HPP
