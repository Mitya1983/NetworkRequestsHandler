#include "network_requests.hpp"
#include "http_header_names.hpp"

#include <asio/io_context.hpp>
#include <asio/connect.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ssl/error.hpp>
#include <asio/ssl/stream.hpp>
#include <asio/read.hpp>
#include <asio/read_until.hpp>

namespace {

const auto readFrame = std::numeric_limits<uint16_t>::max();

} // namespace

void network::process_ssl_network_request(std::shared_ptr<network::Request> request)
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
        request->m_errorDescription = ec.what();
        return;
    }
    asio::ssl::stream<asio::ip::tcp::socket> socket(context, ssl_context);
    asio::error_code error;
    socket.lowest_layer().connect(resolver_results->endpoint(), error);
    if (error){
        request->m_status = Request::Status::ERROR;
        request->m_errorDescription = error.message();
        return;
    }
    socket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    socket.set_verify_mode(asio::ssl::verify_peer);
    socket.handshake(asio::ssl::stream_base::client, error);
    if (error){
        request->m_status = Request::Status::ERROR;
        request->m_errorDescription = error.message();
        return;
    }
    asio::write(socket, asio::buffer(request->request()), error);
    if (error){
        request->m_status = Request::Status::ERROR;
        request->m_errorDescription = error.message();
        return;
    }
    socket.lowest_layer().wait(socket.lowest_layer().wait_read);
    std::string data;
    while (true) {
        char character[1];
        asio::read(socket, asio::buffer(character), asio::transfer_exactly(1), error);
        if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
            request->m_status = Request::Status::ERROR;
            request->m_errorDescription = error.message();
            return;
        }
        data += character[0];
        if (character[0] == '\n' && data.at(data.size() - 3) == '\n'){
            break;
        }
    }
    request->m_reply_base = data;
    data.clear();
    request->_parse_reply_base();

//    asio::read(socket, asio::dynamic_buffer(data), asio::transfer_all(), error);

    if (request->httpStatus() > HttpStatus::IM_Used){
        return;
    }
    auto header = request->find(http::headernames::content_length);
    if (header != request->m_headers.end()){
        request->m_bytesToRead = static_cast<uint64_t>(std::stoull(header->second));
        if (request->m_bytesToRead <= readFrame){
            asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(request->m_bytesToRead), error);
            if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                request->m_status = Request::Status::ERROR;
                request->m_errorDescription = error.message();
                return;
            }
            request->m_bytesRead = request->m_bytesToRead;
        }
        else{
            while (true){
                auto diff = request->m_bytesToRead - request->m_bytesRead;
                if (diff == 0){
                    break;
                }
                if (readFrame <= diff){
                    asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(readFrame), error);
                }
                else{
                    asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(diff), error);
                }
                if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                    request->m_status = Request::Status::ERROR;
                    request->m_errorDescription = error.message();
                    return;
                }
                request->m_bytesRead += readFrame <= diff ? readFrame : diff;
            }
        }
    }
    else{
        header = request->find(http::headernames::transfer_encoding);
        if (header != request->m_headers.end() && header->second.find("chunked") != std::string::npos){
            while (true){
                std::string chunkSize;
                while (true){
                    char character[1];
                    asio::read(socket, asio::buffer(character), asio::transfer_exactly(1), error);
                    if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                        request->m_status = Request::Status::ERROR;
                        request->m_errorDescription = error.message();
                        return;
                    }
                    if (character[0] == '\r'){
                        continue;
                    }
                    if (character[0] == '\n'){
                        break;
                    }
                    chunkSize += character[0];
                }
                auto pos = chunkSize.find(';');
                if (pos != std::string::npos){
                    chunkSize = chunkSize.substr(0, pos);
                }
                uint64_t bytesToRead = std::stoll(chunkSize, 0, 16);
                if (bytesToRead == 0){
                    break;
                }
                asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(bytesToRead), error);
                    if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                        request->m_status = Request::Status::ERROR;
                        request->m_errorDescription = error.message();
                        return;
                    }
                request->m_bytesRead += bytesToRead;
                char skip[2];
                asio::read(socket, asio::buffer(skip), asio::transfer_exactly(2), error);
                if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                    request->m_status = Request::Status::ERROR;
                    request->m_errorDescription = error.message();
                    return;
                }
            }
        }
        else {
            //TODO: Decide what to do here;
        }
    }
    request->m_reply_body = data;
    socket.lowest_layer().cancel();
    socket.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.lowest_layer().close();
    request->m_status = Request::Status::DONE;
}
