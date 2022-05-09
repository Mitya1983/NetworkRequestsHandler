#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "network_request.hpp"
#include "status_codes.hpp"
#include "url.hpp"
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

    /**
     * \class HttpRequest
     * \extends NetworkRequest<HttpResponse>
     * \brief Base class for http requests
     */
    class HttpRequest : public NetworkRequest<HttpResponse>{
      public:
        /**
         * \brief Adds header and corresponding value.
         * \param header const std::string&. Common headers are listen in \file http_header_names.hpp
         * \param value const std::string&
         */
        void addHeader(const std::string& header, const std::string& value);
        /**
         * \brief Adds param and corresponding value.
         * \param param_name const std::string&
         * \param param_value const std::string&
         */
        void addParam(const std::string& param_name, const std::string& param_value = "");
        /**
         * \brief Sets output directory. Applicable if outputToFile() is invoked.
         * \param directory const std::filesystem::path&
         */
        void outputToDirectory(const std::filesystem::path& directory);
        /**
         * \brief Directs output to specified file. If filename is not provided the filename from url is used.
         * \param filename const std::string&.
         */
        void outputToFile(const std::string& filename = "");
        /**
         * \implements NetworkRequest::doRequest()
         * \brief Implements request processing.
         */
        void doRequest() override;

      protected:
        /**
         * \brief Constructor
         * \param uri Uri
         */
        explicit HttpRequest(Url uri);
        ~HttpRequest() override = default;
        std::unordered_map<std::string, std::string> m_headers;
        std::unordered_map<std::string, std::string> m_params;
        template<class Socket> void _read(Socket& socket, HttpResponse& response, std::error_code& error);
        void _doHttpRequest();
        void _doHttpsRequest();
    };

    /**
     * \class GetRequest
     * \extends HttpRequest
     * \brief Handles GET http requests
     */
    class GetRequest : public HttpRequest{
      public:
        /**
         * \brief Constructor
         * \param uri Uri
         */
        explicit GetRequest(Url uri);
        ~GetRequest() override = default;

        /**
         * \brief Prepares string representation of the request.
         * \implements NetworkRequest::prepareRequest()
         * \return std::string
         */
        auto prepareRequest() const -> std::string override;
    };

    /**
    * \class PostRequest
    * \extends HttpRequest
    * \brief Handles POST http requests
    */
    class PostRequest : public HttpRequest{
      public:
        /**
         * \brief Constructor
         * \param uri Uri
         */
        explicit PostRequest(Url uri);
        ~PostRequest() override = default;

        /**
        * \brief Prepares string representation of the request.
        * \implements NetworkRequest::prepareRequest()
        * \return std::string
        */
        auto prepareRequest() const -> std::string override;
    };

} // namespace network

#endif // HTTP_REQUEST_H
