#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "network_request_base.hpp"
#include "http_status_codes.hpp"
#include "url.hpp"
#include "http_header.hpp"
#include "http_param.hpp"
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
     * \extends TcpRequest
     * \brief Base class for http requests
     */
    class HttpRequest : public NetworkRequestBase {
      public:
        /**
         * \brief Adds header and corresponding value.
         * \param header const std::string&. Common headers are listen in \file http_header_names.hpp
         * \param value const std::string&
         */
        void addHeader(Header&& header);
        /**
         * \brief Adds param and corresponding value.
         * \param param_name const std::string&
         * \param param_value const std::string&
         */
        void addParam(Parameter&& parameter);

        void initResponse(std::vector<uint8_t>&& headers_data);
      protected:

        /**
         * \brief Constructor
         * \param url url
         */
        explicit HttpRequest(Url&& url);
        explicit HttpRequest(const Url& url);
        ~HttpRequest() override = default;
        HttpHeaders m_headers;
        HttpParams m_params;
        bool m_request_composed;
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
         * \param url url
         */
        explicit GetRequest(Url&& url);
        explicit GetRequest(const Url& url);
        ~GetRequest() override = default;

        /**
         * \brief Prepares std::vector<uint8_t> representation of the request.
         * \implements TcpRequest::requestData()
         * \return const std::vector<uint8_t>&
         */
        auto requestData() -> const std::vector<uint8_t>& override;
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
         * \param url url
         */
        explicit PostRequest(Url&& url);
        explicit PostRequest(const Url& url);
        ~PostRequest() override = default;

        void setBody(std::string&& p_body);
        void setBody(const std::string& p_body);

        /**
        * \brief Prepares std::vector<uint8_t> representation of the request.
        * \implements TcpRequest::requestData()
        * \return const std::vector<uint8_t>&
        */
        auto requestData() -> const std::vector<uint8_t>& override;
    protected:
        std::string m_body;
    };


    /**
    * \class PutRequest
    * \extends HttpRequest
    * \brief Handles POST http requests
    */
    class PutRequest : public PostRequest{
    public:
        /**
         * \brief Constructor
         * \param url url
         */
        explicit PutRequest(Url&& url);
        explicit PutRequest(const Url& url);
        ~PutRequest() override = default;

        /**
        * \brief Prepares std::vector<uint8_t> representation of the request.
        * \implements TcpRequest::requestData()
        * \return const std::vector<uint8_t>&
        */
        auto requestData() -> const std::vector<uint8_t>& override;
    };


} // namespace network

#endif // HTTP_REQUEST_H
