#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "network_request.hpp"
#include "status_codes.hpp"
#include "uri.hpp"
#include "http_response.hpp"
#include "network_utility.hpp"
#include "http_header_names.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <filesystem>
#include <optional>
#include <fstream>

namespace tristan::network{

    class HttpRequest : public NetworkRequest<HttpResponse>{
      public:
        void addHeader(const std::string& header, const std::string& value);
        void addParam(const std::string& paramName, const std::string& paramValue = "");
        void outputToDirectory(const std::filesystem::path& directory);
        void outputToFile(const std::string& filename = "");
        void doRequest() override;

      protected:
        explicit HttpRequest(Uri uri);
        ~HttpRequest() override = default;
        std::unordered_map<std::string, std::string> m_headers;
        std::unordered_map<std::string, std::string> m_params;
        template<class Socket> void _read(Socket& socket, HttpResponse& response, std::error_code& error);
        void _doHttpRequest();
        void _doHttpsRequest();
    };

    class GetRequest : public HttpRequest{
      public:
        explicit GetRequest(Uri uri);
        ~GetRequest() override = default;

      protected:
        auto prepareRequest() const -> std::string override;
    };

    class PostRequest : public HttpRequest{
      public:
        explicit PostRequest(Uri uri);
        ~PostRequest() override = default;

      protected:
        auto prepareRequest() const -> std::string override;
    };

} // namespace network

#endif // HTTP_REQUEST_H
