#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>


namespace zestad::network {

class Request
{
    friend void process_ssl_network_request(std::shared_ptr<Request> request);

public:
    enum class Status : uint8_t {WAITING, PROCESSED, DONE, ERROR};
protected:

    //CONSTRUCTORS
    Request(const std::string &url, const std::string &descritpion);
    Request(const Request&) = delete;
    Request(Request&&) = default;
    //OPERATORS
    Request& operator=(const Request&) = delete;
    Request& operator=(Request&&) = default;
    //DESTRUCTOR
    ~Request() = default;

    std::string m_description;
    std::string m_host;
    std::string m_requestUrl;
    std::string m_error;

    std::string m_reply_base;
    std::string m_reply_body;
    std::unordered_map<std::string, std::string> m_params;
    std::unordered_map<std::string, std::string> m_headers;
    Status m_status;

    //API
    std::string _get_host_from_url(const std::string &url);
    std::string _get_request_from_url(const std::string &url);
    void _set_base(const std::string &base) {m_reply_base = base;}
    void _set_body(const std::string &body) {m_reply_body = body;}
    void _set_error(const std::string &error) {m_error = error;}
    void _set_status(Status status) {m_status = status;}
    //SETTERS AND GETTERS
public:
    void add_header(const std::string &header, const std::string &value);
    void add_param(std::string paramName, const std::string &paramValue = "");
    [[nodiscard]] const std::string &base() const noexcept {return m_reply_base;}
    [[nodiscard]] const std::string &body() const noexcept {return m_reply_body;}

    const std::string &host() const noexcept {return m_host;}
    const std::string &description() const noexcept {return m_description;}
    const std::string &error() const noexcept {return m_error;}
    Status status() const noexcept {return m_status;}

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

} // namespace zestad::network

#endif // HTTPREQUEST_H
