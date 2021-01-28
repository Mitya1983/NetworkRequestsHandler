#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "status_codes.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>



namespace network {


class Request
{
    friend void process_ssl_network_request(std::shared_ptr<Request> request);

public:
    enum class Status : uint8_t {WAITING, PROCESSED, DONE, REDIRECT, ERROR};
protected:
    //CONSTRUCTORS
    Request(const std::string &url, const std::string &descritpion = "");
    Request(const Request&) = delete;
    Request(Request&&) = default;
    //OPERATORS
    Request& operator=(const Request&) = delete;
    Request& operator=(Request&&) = default;
    //DESTRUCTOR
    ~Request() = default;

    std::unordered_map<std::string, std::string> m_params;
    std::vector<std::pair<std::string, std::string>> m_headers;

    std::string m_description;
    std::string m_host;
    std::string m_requestUrl;
    std::string m_errorDescription;

    std::string m_reply_base;
    std::string m_reply_body;

    uint64_t m_bytesToRead;
    uint64_t m_bytesRead;

    Status m_status;
    HttpStatus m_httpStatus;

    //API
    std::string _get_host_from_url(const std::string &url);
    std::string _get_request_from_url(const std::string &url);
    void _set_base(const std::string &base) {m_reply_base = base;}
    void _set_body(const std::string &body) {m_reply_body = body;}
    void _set_status(Status status) {m_status = status;}
    void _parse_reply_base();
    //SETTERS AND GETTERS
public:
    void add_header(const std::string &header, const std::string &value);
    void add_param(const std::string &paramName, const std::string &paramValue = "");
    const std::string &base() const noexcept {return m_reply_base;}

    const std::vector<std::pair<std::string, std::string>> headers() const noexcept {return m_headers;}
    std::vector<std::pair<std::string, std::string>>::const_iterator find(const std::string headerName) const;

    const std::string &body() const noexcept {return m_reply_body;}

    const std::string &host() const noexcept {return m_host;}
    const std::string &description() const noexcept {return m_description;}
    const std::string &errorDescription() const noexcept {return m_errorDescription;}

    uint64_t bytesToRead() const noexcept {return m_bytesToRead;}
    uint64_t bytesRead() const noexcept {return m_bytesRead;}

    Status status() const noexcept {return m_status;}
    HttpStatus httpStatus() const noexcept {return m_httpStatus;}
    virtual std::string request() const = 0;

};

class GetRequest : public Request
{
public:

    //CONSTRUCTORS
    GetRequest(const std::string &url, const std::string &descritpion);
    GetRequest(const GetRequest&) = delete;
    GetRequest(GetRequest&&) = default;
    //OPERATORS
    GetRequest& operator=(const GetRequest&) = delete;
    GetRequest& operator=(GetRequest&&) = default;
    //DESTRUCTOR
    ~GetRequest() = default;

    //API
private:
    //SETTERS AND GETTERS
public:
    std::string request() const override;
};

class PostRequest : public Request
{
public:
    //CONSTRUCTORS
    PostRequest(const std::string &url, const std::string &descritpion);
    PostRequest(const PostRequest&) = delete;
    PostRequest(PostRequest&&) = default;
    //OPERATORS
    PostRequest& operator=(const PostRequest&) = delete;
    PostRequest& operator=(PostRequest&&) = default;
    //DESTRUCTOR
    ~PostRequest() = default;

    //API


protected:

private:

    //SETTERS AND GETTERS
public:
    std::string request() const override;
};

} // namespace network

#endif // HTTPREQUEST_H
