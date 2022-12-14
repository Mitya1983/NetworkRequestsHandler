#include "http_request.hpp"
#include "http_header_names.hpp"
#include "http_response.hpp"
#include "network_utility.hpp"
#include "network_logger.hpp"

//#include "asio/io_context.hpp"
//#include "asio/connect.hpp"
//#include "asio/read.hpp"
//#include "asio/ssl/error.hpp"
//#include "asio/ip/tcp.hpp"
//#include "asio/ssl/stream.hpp"
//
//#include <thread>
//#include <array>
//#include <fstream>
//#include <utility>

tristan::network::HttpRequest::HttpRequest(Url&& url) :
    NetworkRequestBase(std::move(url)),
    m_request_composed(false) {
    if (not m_url.isValid() || (m_url.scheme() != "http" && m_url.scheme() != "https" && m_url.port() != "80" && m_url.port() != "443")) {
        netError("Invalid url for HTTP request received");
        tristan::network::NetworkRequestBase::setError(tristan::network::makeError(tristan::network::ErrorCode::INVALID_URL));
        return;
    }
    m_headers.addHeader(tristan::network::Header(tristan::network::http::header_names::host, m_url.host()));
    if (m_url.portUint16_t_local_byte_order() == 443) {
        m_ssl = true;
    }
}

tristan::network::HttpRequest::HttpRequest(const tristan::network::Url& url) :
    HttpRequest(Url(url)) { }

void tristan::network::HttpRequest::addHeader(tristan::network::Header&& header) {
    if (header.name.empty()) {
        return;
    }
    m_headers.addHeader(std::move(header));
}

void tristan::network::HttpRequest::addParam(tristan::network::Parameter&& parameter) {
    if (parameter.name.empty()) {
        return;
    }
    m_params.addParameter(std::move(parameter));
}

void tristan::network::HttpRequest::initResponse(std::vector< uint8_t >&& headers_data) {
    m_response = tristan::network::HttpResponse::createResponse(m_uuid, std::move(headers_data));
}

tristan::network::GetRequest::GetRequest(Url&& url) :
    HttpRequest(std::move(url)) { }

tristan::network::GetRequest::GetRequest(const tristan::network::Url& url) :
    HttpRequest(Url(url)) { }

auto tristan::network::GetRequest::requestData() -> const std::vector< uint8_t >& {
    if (not m_request_composed) {
        std::string to_insert("GET ");
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());
        if (m_url.path().empty() || m_url.path().at(0) != '/') {
            m_request_data.push_back('/');
        }
        m_request_data.insert(m_request_data.end(), m_url.path().begin(), m_url.path().end());
        if (!m_params.empty()) {
            m_request_data.push_back('?');
            int param_count = 0;
            for (const auto& param: m_params) {
                if (param_count > 0) {
                    m_request_data.push_back('&');
                }
                m_request_data.insert(m_request_data.end(), param.name.begin(), param.name.end());
                if (!param.value.empty()) {
                    m_request_data.push_back('=');
                    m_request_data.insert(m_request_data.end(), param.value.begin(), param.value.end());
                }
                ++param_count;
            }
        }
        if (!m_url.query().empty()) {
            m_request_data.push_back('&');
            m_request_data.insert(m_request_data.end(), m_url.query().begin(), m_url.query().end());
        }
        to_insert = " HTTP/1.1\r\n";
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());
        if (!m_headers.empty()) {
            for (const auto& header: m_headers) {
                m_request_data.insert(m_request_data.end(), header.name.begin(), header.name.end());
                m_request_data.push_back(':');
                m_request_data.insert(m_request_data.end(), header.value.begin(), header.value.end());
                m_request_data.push_back('\r');
                m_request_data.push_back('\n');
            }
        }
        m_request_data.push_back('\r');
        m_request_data.push_back('\n');
        m_request_data.shrink_to_fit();
        m_request_composed = true;
    }
    return m_request_data;
}

tristan::network::PostRequest::PostRequest(Url&& url) :
    HttpRequest(std::move(url)) { }

tristan::network::PostRequest::PostRequest(const tristan::network::Url& url) :
    HttpRequest(Url(url)) { }

auto tristan::network::PostRequest::requestData() -> const std::vector< uint8_t >& {
    if (not m_request_composed) {
        std::string body;
        if (!m_params.empty()) {
            int param_count = 0;
            for (const auto& param: m_params) {
                if (param_count > 0) {
                    body += '&';
                }
                body += param.name;
                body += '=';
                auto content_type = m_headers.headerValue(tristan::network::http::header_names::content_type);
                if (content_type && content_type.value() == "application/x-www-form-urlencoded") {
                    body += tristan::network::utility::encodeUrl(param.value);
                } else if (content_type && content_type.value() == "multipart/form-data") {
                    //NOTE: To be developed in following versions
                } else {
                    body += param.value;
                }
                ++param_count;
            }
        }
        m_headers.addHeader(tristan::network::Header(tristan::network::http::header_names::content_length, std::to_string(body.size())));
        std::string to_insert = "POST ";
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());
        if (m_url.path().empty() || m_url.path().at(0) != '/') {
            m_request_data.push_back('/');
        }
        m_request_data.insert(m_request_data.end(), m_url.path().begin(), m_url.path().end());
        to_insert = " HTTP/1.1\r\n";
        m_request_data.insert(m_request_data.end(), to_insert.begin(), to_insert.end());

        if (!m_headers.empty()) {
            for (const auto& header: m_headers) {
                m_request_data.insert(m_request_data.end(), header.name.begin(), header.name.end());
                m_request_data.push_back(':');
                m_request_data.insert(m_request_data.end(), header.value.begin(), header.value.end());
                m_request_data.push_back('\r');
                m_request_data.push_back('\n');
            }
        }
        m_request_data.push_back('\r');
        m_request_data.push_back('\n');
        m_request_data.push_back('\r');
        m_request_data.push_back('\n');

        if (!body.empty()) {
            m_request_data.insert(m_request_data.end(), body.begin(), body.end());
        }
        m_request_data.shrink_to_fit();
        m_request_composed = true;
    }
    return m_request_data;
}

// template<class Socket> void tristan::network::HttpRequest::_read(Socket& socket, tristan::network::HttpResponse& response, std::error_code& error){
//     std::string data;
//     if (m_output_to_file){
//         tristan::network::utility::checkFileName(m_output_path);
//         m_output_path.replace_filename(m_output_path.filename().string() + ".part");
//         m_output_file.open(m_output_path, std::ios::binary | std::ios::app);
//         if (!m_output_file.is_open()){
//             m_status = tristan::network::Status::ERROR;
//             m_error = std::error_code(errno, std::system_category());
//             _notifyWhenFinished();
//             return;
//         }
//     }
//     auto content_length = response.m_headers.find(tristan::network::http::header_names::content_length);
//     if (content_length != response.m_headers.end()){
//         m_bytes_to_read = static_cast<uint64_t>(std::stoll(content_length->second));
//         if (m_bytes_to_read != 0 && m_bytes_to_read <= m_read_frame){
//             asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(m_bytes_to_read), error);
//              if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
//                 m_status = tristan::network::Status::ERROR;
//                 m_error = error;
//                 _notifyWhenFinished();
//                 return;
//             }
//             m_bytes_read = m_bytes_to_read;
//             _notifyWhenBytesReadChanged();
//             if (m_output_to_file){
//                 m_output_file << data;
//             }
//         }
//         else{
//             while (true){
//                 if (m_paused){
//                     std::this_thread::sleep_for(std::chrono::seconds(1));
//                     continue;
//                 }
//                 auto diff = m_bytes_to_read - m_bytes_read;
//                 if (diff == 0 || m_cancelled){
//                     break;
//                 }
//                 if (m_read_frame <= diff){
//                     asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(m_read_frame), error);
//                 }
//                 else{
//                     asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(diff), error);
//                 }
//                 if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
//                     m_status = tristan::network::Status::ERROR;
//                     m_error = error;
//                     _notifyWhenFinished();
//                     return;
//                 }
//                 m_bytes_read += m_read_frame <= diff ? m_read_frame : diff;
//                 _notifyWhenBytesReadChanged();
//                 if (m_output_to_file){
//                     m_output_file << data;
//                 }
//             }
//         }
//     }
//     else{
//         auto transfer_encoding = response.m_headers.find(tristan::network::http::header_names::transfer_encoding);
//         if (transfer_encoding != response.m_headers.end()){
//             while (true){
//                 if (m_paused){
//                     std::this_thread::sleep_for(std::chrono::seconds(1));
//                     continue;
//                 }
//                 std::string chunk_size;
//                 while (true){
//                     std::array<char, 1> character{ };
//                     asio::read(socket, asio::buffer(character), asio::transfer_exactly(1), error);
//                     if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
//                         m_status = tristan::network::Status::ERROR;
//                         m_error = error;
//                         _notifyWhenFinished();
//                         return;
//                     }
//                     if (character[0] == '\r'){
//                         continue;
//                     }
//                     if (character[0] == '\n'){
//                         break;
//                     }
//                     chunk_size += character[0];
//                 }
//                 auto pos = chunk_size.find(';');
//                 if (pos != std::string::npos){
//                     chunk_size = chunk_size.substr(0, pos);
//                 }
//                 uint64_t bytes_to_read = std::stoll(chunk_size, nullptr, 16);
//                 if (bytes_to_read == 0 || m_cancelled){
//                     break;
//                 }
//                 asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(bytes_to_read), error);
//                 if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
//                     m_status = tristan::network::Status::ERROR;
//                     m_error = error;
//                     _notifyWhenFinished();
//                     return;
//                 }
//                 m_bytes_read += bytes_to_read;
//                 _notifyWhenBytesReadChanged();
//                 if (m_output_to_file){
//                     m_output_file << data;
//                 }
//                 std::array<char, 2> skip{ };
//                 asio::read(socket, asio::buffer(skip), asio::transfer_exactly(2), error);
//                 if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
//                     m_status = tristan::network::Status::ERROR;
//                     m_error = error;
//                     _notifyWhenFinished();
//                     return;
//                 }
//             }
//         }
//     }
//     if (!data.empty() && !m_output_to_file){
//         response.m_data = data;
//     }
//     if (m_output_to_file){
//         m_output_file.close();
//         if (!m_cancelled){
//             auto temp_file_name = m_output_path;
//             m_output_path.replace_extension("");
//             std::filesystem::rename(temp_file_name, m_output_path);
//         }
//         else{
//             std::filesystem::remove(m_output_path);
//         }
//     }
// void tristan::network::HttpRequest::_doHttpRequest(){
//     asio::io_context context;
//     asio::ip::tcp::resolver resolver(context);
//     asio::ip::basic_resolver_results<asio::ip::tcp> resolver_results;
//     try{
//         resolver_results = resolver.resolve(m_url.host(), m_url.port());
//     }
//     catch (const asio::system_error& error){
//         m_status = tristan::network::Status::ERROR;
//         m_error = error.code();
//         _notifyWhenError();
//         return;
//     }
//     asio::ip::tcp::socket socket(context);
//     asio::error_code error;
//     socket.connect(resolver_results->endpoint(), error);
//     if (error){
//         m_status = tristan::network::Status::ERROR;
//         m_error = error;
//         _notifyWhenError();
//         return;
//     }
//     socket.set_option(asio::ip::tcp::no_delay(true));
//     asio::write(socket, asio::buffer(this->prepareRequest()), error);
//     if (error){
//         m_status = tristan::network::Status::ERROR;
//         m_error = error;
//         _notifyWhenError();
//         return;
//     }
//     socket.wait(socket.lowest_layer().wait_read);
//     std::string response_header;
//     while (true){
//         std::array<char, 1> character{ };
//         asio::read(socket, asio::buffer(character), asio::transfer_exactly(1), error);
//         if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
//             m_status = tristan::network::Status::ERROR;
//             m_error = error;
//             _notifyWhenError();
//             return;
//         }
//         response_header += character[0];
//         if (character[0] == '\n' && response_header.at(response_header.size() - 3) == '\n'){
//             break;
//         }
//     }
//     tristan::network::HttpResponse response(response_header, m_uuid);
//     auto content_length = response.headerValue(tristan::network::http::header_names::content_length);
//     if (response.m_status >= tristan::network::HttpStatus::IM_Used && (content_length == std::nullopt || content_length->empty() || content_length == "0")){
//         m_response = std::make_shared<tristan::network::HttpResponse>(std::move(response));
//         m_status = tristan::network::Status::DONE;
//         _notifyWhenFinished();
//         return;
//     }
//     _read(socket, response, error);
//
//     socket.lowest_layer().cancel();
//     socket.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both);
//     socket.lowest_layer().close();
//     m_status = tristan::network::Status::DONE;
//     m_response = std::make_shared<tristan::network::HttpResponse>(std::move(response));
//     _notifyWhenFinished();
// }
//
// void tristan::network::HttpRequest::_doHttpsRequest(){
//     asio::io_context context;
//     asio::ssl::context ssl_context(asio::ssl::context::sslv23_client);
//     ssl_context.set_default_verify_paths();
//     ssl_context.set_verify_mode(asio::ssl::verify_peer);
//     asio::ip::tcp::resolver resolver(context);
//     asio::ip::basic_resolver_results<asio::ip::tcp> resolver_results;
//     try{
//         resolver_results = resolver.resolve(m_url.host(), m_url.port());
//     }
//     catch (const asio::system_error& error){
//         m_status = tristan::network::Status::ERROR;
//         m_error = error.code();
//         _notifyWhenError();
//         return;
//     }
//     asio::ssl::stream<asio::ip::tcp::socket> socket(context, ssl_context);
//     asio::error_code error;
//     socket.lowest_layer().connect(resolver_results->endpoint(), error);
//     if (error){
//         m_status = tristan::network::Status::ERROR;
//         m_error = error;
//         _notifyWhenError();
//         return;
//     }
//     socket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
//     socket.set_verify_mode(asio::ssl::verify_peer);
//     SSL_set_tlsext_host_name(socket.native_handle(), m_url.host().c_str());
//     socket.handshake(asio::ssl::stream_base::client, error);
//     if (error){
//         m_status = tristan::network::Status::ERROR;
//         m_error = error;
//         _notifyWhenError();
//         return;
//     }
//     asio::write(socket, asio::buffer(this->prepareRequest()), error);
//     if (error){
//         m_status = tristan::network::Status::ERROR;
//         m_error = error;
//         _notifyWhenError();
//         return;
//     }
//     socket.lowest_layer().wait(socket.lowest_layer().wait_read);
//     std::string response_header;
//     while (true){
//         std::array<char, 1> character{ };
//         asio::read(socket, asio::buffer(character), asio::transfer_exactly(1), error);
//         if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
//             m_status = tristan::network::Status::ERROR;
//             m_error = error;
//             _notifyWhenError();
//             return;
//         }
//         response_header += character[0];
//         if (character[0] == '\n' && response_header.at(response_header.size() - 3) == '\n'){
//             break;
//         }
//     }
//     tristan::network::HttpResponse response(response_header, m_uuid);
//     auto content_length = response.headerValue(tristan::network::http::header_names::content_length);
//     if (response.m_status >= tristan::network::HttpStatus::IM_Used && (content_length == std::nullopt || content_length->empty() || content_length == "0")){
//         m_response = std::make_shared<tristan::network::HttpResponse>(std::move(response));
//         m_status = tristan::network::Status::DONE;
//         _notifyWhenFinished();
//         return;
//     }
//
//     _read(socket, response, error);
//     socket.lowest_layer().cancel();
//     socket.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both);
//     socket.lowest_layer().close();
//     m_status = tristan::network::Status::DONE;
//     m_response = std::make_shared<tristan::network::HttpResponse>(std::move(response));
//     _notifyWhenFinished();
// }
// }
