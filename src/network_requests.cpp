#include "network_requests.hpp"

//#include <linux/inc/log.hpp>

#include <asio/io_context.hpp>
#include <asio/connect.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ssl/error.hpp>
#include <asio/ssl/stream.hpp>
#include <asio/read.hpp>

namespace {

std::string httpResponseCode(const std::string & httpResponse);
std::string httpResponseDescription(const std::string & httpResponse);

} // namespace name

void zestad::network::process_ssl_network_request(std::shared_ptr<zestad::network::Request> request)
{
    request->m_status = Request::Status::PROCESSED;
    asio::io_context context;
    asio::ssl::context ssl_context(asio::ssl::context::sslv23_client);
    ssl_context.set_default_verify_paths();
    ssl_context.set_verify_mode(asio::ssl::verify_peer);
    asio::ip::tcp::resolver resolver(context);
    asio::ip::basic_resolver_results<asio::ip::tcp> resolver_results;
    try {
        resolver_results = resolver.resolve(request->host(), "443");
    }  catch (const asio::system_error &ec) {
        request->m_status = Request::Status::ERROR;
        request->m_error = ec.what();
        return;
    }
    asio::ssl::stream<asio::ip::tcp::socket> socket(context, ssl_context);
    asio::error_code error;
    socket.lowest_layer().connect(resolver_results->endpoint(), error);
    if (error){
        request->m_status = Request::Status::ERROR;
        request->m_error = error.message();
        return;
    }
    socket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    socket.set_verify_mode(asio::ssl::verify_peer);
    socket.handshake(asio::ssl::stream_base::client, error);
    if (error){
        request->m_status = Request::Status::ERROR;
        request->m_error = error.message();
        return;
    }
    asio::write(socket, asio::buffer(request->request()), error);
    if (error){
        request->m_status = Request::Status::ERROR;
        request->m_error = error.message();
        return;
    }
    socket.lowest_layer().wait(socket.lowest_layer().wait_read);
    std::string data;
    asio::read(socket, asio::dynamic_buffer(data), asio::transfer_all(), error);
    if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
        request->m_status = Request::Status::ERROR;
        request->m_error = error.message();
        return;
    }
    socket.lowest_layer().cancel();
    socket.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.lowest_layer().close();
    auto baseEnd = data.find("\r\n\r\n");
    request->m_reply_base = data.substr(0, data.find("\r\n\r\n"));
    if (httpResponseCode(request->m_reply_base) != "200"){
        request->m_status = Request::Status::ERROR;
        request->m_error = httpResponseDescription(data);
        return;
    }
    baseEnd += 4;
    auto bodyStart = data.find("\r\n", baseEnd) + 3;
    unsigned long numberOfBytes = 0;
    if (request->m_reply_base.find(" chunked") != std::string::npos){
        auto bodyEnd = data.find("\0\r\n");
        numberOfBytes = bodyEnd - bodyStart;
    }
    else if (auto contentLength = request->m_reply_base.find("Content-Length") != std::string::npos){
        contentLength += 16;
        auto contentLengthEnd = request->m_reply_base
.find("\r\n", contentLength);
        std::string length = request->m_reply_base.substr(contentLength, contentLengthEnd - contentLength);
        numberOfBytes = std::stol(length);
    }
    request->m_reply_body = data.substr(baseEnd, numberOfBytes);
    request->m_status = Request::Status::DONE;
//    Log::write(request->description() + " was succesfuly processed");
}


namespace {

std::string httpResponseCode(const std::string & httpResponse){
    return httpResponse.substr(9, 3);
}
std::string httpResponseDescription(const std::string & httpResponse){
    auto endPos = httpResponse.find("\r\n");
    return httpResponse.substr(13, endPos - 13);
}
} // namespace name
