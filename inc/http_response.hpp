#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "network_response.hpp"
#include "http_header.hpp"
#include "http_param.hpp"

namespace tristan::network {

    class HttpResponse : public NetworkResponse
    {
        friend class NetworkRequest;
        friend class HttpRequest;
    public:
        HttpResponse() = delete;
        HttpResponse(const HttpResponse& other) = delete;
        HttpResponse(HttpResponse&& other) = default;

        HttpResponse& operator=(const HttpResponse& other) = delete;
        HttpResponse& operator=(HttpResponse&& other) = default;

        ~HttpResponse() override = default;

        [[nodiscard]] static auto createResponse(std::string uuid, std::vector<uint8_t>&& headers_data) -> std::shared_ptr<HttpResponse>;

        [[nodiscard]] auto headers() const -> const std::unique_ptr<HttpHeaders>&;

    protected:
    private:
        explicit HttpResponse(std::string&& uuid, std::vector<uint8_t>&& headers_data);

        std::unique_ptr<HttpHeaders> m_response_headers;

    };

} //End of tristan::network namespace

#endif  //HTTP_RESPONSE_HPP
