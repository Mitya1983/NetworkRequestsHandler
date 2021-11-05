#include "http_request.hpp"

#include "asio/io_context.hpp"
#include "asio/connect.hpp"
#include "asio/read.hpp"
#include "asio/ssl/error.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/ssl/stream.hpp"

#include <thread>
#include <array>
#include <fstream>
#include <utility>

namespace{
    const uint16_t read_frame = std::numeric_limits<uint16_t>::max();
} //End of unknown namespace

tristan::network::HttpRequest::HttpRequest(Uri uri) :
        NetworkRequest(std::move(uri)){
    if (!m_uri.isValid() || (m_uri.scheme() != "http" && m_uri.scheme() != "https")){
        m_status = tristan::network::Status::ERROR;
        m_error = asio::error::make_error_code(asio::error::basic_errors::fault);
        _notifyWhenError();
        return;
    }
    m_headers.emplace(tristan::network::http::header_names::host, m_uri.host());
    if (m_uri.port().empty()){
        if (m_uri.scheme() == "http"){
            m_uri.setPort(80);
        }
        else{
            m_uri.setPort(433);
        }
    }
}

void tristan::network::HttpRequest::addHeader(const std::string& header, const std::string& value){
    if (header.empty()){
        return;
    }
    m_headers.emplace(header, value);
}

void tristan::network::HttpRequest::addParam(const std::string& paramName, const std::string& paramValue){
    if (paramName.empty()){
        return;
    }
    m_params.emplace(paramName, paramValue);
}

void tristan::network::HttpRequest::outputToDirectory(const std::filesystem::path& directory){
    if (directory.empty()){
        return;
    }
    m_output_path = directory;
    if (!std::filesystem::exists(m_output_path)){
        try{
            std::filesystem::create_directories(m_output_path);
        }
        catch (const std::filesystem::filesystem_error& e){
            throw std::runtime_error(e.what());
        }
    }
}

void tristan::network::HttpRequest::outputToFile(const std::string& filename){
    m_output_to_file = true;
    if (filename.empty()){
        m_output_path.append(std::filesystem::path(m_uri.path()).filename().string());
        if (m_output_path.filename().empty()){
            m_output_path.append("New_download");
        }
    }
    else{
        m_output_path.append(filename);
    }
}

void tristan::network::HttpRequest::doRequest(){

    m_status = tristan::network::Status::PROCESSED;
    if (m_uri.scheme() == "https"){
        _doHttpsRequest();
    }
    else{
        _doHttpRequest();
    }
}

template<class Socket> void tristan::network::HttpRequest::_read(Socket& socket, tristan::network::HttpResponse& response, std::error_code& error){
    std::string data;
    if (m_output_to_file){
        tristan::network::utility::checkFileName(m_output_path);
        m_output_path.replace_filename(m_output_path.filename().string() + ".part");
        m_output_file.open(m_output_path, std::ios::binary | std::ios::app);
        if (!m_output_file.is_open()){
            m_status = tristan::network::Status::ERROR;
            m_error = std::error_code(errno, std::system_category());
            _notifyWhenFinished();
            return;
        }
    }
    auto content_length = response.m_headers.find(tristan::network::http::header_names::content_length);
    if (content_length != response.m_headers.end()){
        m_bytes_to_read = static_cast<uint64_t>(std::stoll(content_length->second));
        if (m_bytes_to_read != 0 && m_bytes_to_read <= m_read_frame){
            asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(m_bytes_to_read), error);
            if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                m_status = tristan::network::Status::ERROR;
                m_error = error;
                _notifyWhenFinished();
                return;
            }
            m_bytes_read = m_bytes_to_read;
            _notifyWhenBytesReadChanged();
            if (m_output_to_file){
                m_output_file << data;
            }
        }
        else{
            while (true){
                if (m_paused){
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
                auto diff = m_bytes_to_read - m_bytes_read;
                if (diff == 0 || m_cancelled){
                    break;
                }
                if (m_read_frame <= diff){
                    asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(m_read_frame), error);
                }
                else{
                    asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(diff), error);
                }
                if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                    m_status = tristan::network::Status::ERROR;
                    m_error = error;
                    _notifyWhenFinished();
                    return;
                }
                m_bytes_read += m_read_frame <= diff ? m_read_frame : diff;
                _notifyWhenBytesReadChanged();
                if (m_output_to_file){
                    m_output_file << data;
                }
            }
        }
    }
    else{
        auto transfer_encoding = response.m_headers.find(tristan::network::http::header_names::transfer_encoding);
        if (transfer_encoding != response.m_headers.end()){
            while (true){
                if (m_paused){
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
                std::string chunk_size;
                while (true){
                    std::array<char, 1> character{ };
                    asio::read(socket, asio::buffer(character), asio::transfer_exactly(1), error);
                    if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                        m_status = tristan::network::Status::ERROR;
                        m_error = error;
                        _notifyWhenFinished();
                        return;
                    }
                    if (character[0] == '\r'){
                        continue;
                    }
                    if (character[0] == '\n'){
                        break;
                    }
                    chunk_size += character[0];
                }
                auto pos = chunk_size.find(';');
                if (pos != std::string::npos){
                    chunk_size = chunk_size.substr(0, pos);
                }
                uint64_t bytes_to_read = std::stoll(chunk_size, nullptr, 16);
                if (bytes_to_read == 0 || m_cancelled){
                    break;
                }
                asio::read(socket, asio::dynamic_buffer(data), asio::transfer_exactly(bytes_to_read), error);
                if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                    m_status = tristan::network::Status::ERROR;
                    m_error = error;
                    _notifyWhenFinished();
                    return;
                }
                m_bytes_read += bytes_to_read;
                _notifyWhenBytesReadChanged();
                if (m_output_to_file){
                    m_output_file << data;
                }
                std::array<char, 2> skip{ };
                asio::read(socket, asio::buffer(skip), asio::transfer_exactly(2), error);
                if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
                    m_status = tristan::network::Status::ERROR;
                    m_error = error;
                    _notifyWhenFinished();
                    return;
                }
            }
        }
    }
    if (!data.empty() && !m_output_to_file){
        response.m_data = data;
    }
    if (m_output_to_file){
        m_output_file.close();
        if (!m_cancelled){
            auto temp_file_name = m_output_path;
            m_output_path.replace_extension("");
            std::filesystem::rename(temp_file_name, m_output_path);
        }
        else{
            std::filesystem::remove(m_output_path);
        }
    }
}

void tristan::network::HttpRequest::_doHttpRequest(){
    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    asio::ip::basic_resolver_results<asio::ip::tcp> resolver_results;
    try{
        resolver_results = resolver.resolve(m_uri.host(), m_uri.port());
    }
    catch (const asio::system_error& error){
        m_status = tristan::network::Status::ERROR;
        m_error = error.code();
        _notifyWhenError();
        return;
    }
    asio::ip::tcp::socket socket(context);
    asio::error_code error;
    socket.connect(resolver_results->endpoint(), error);
    if (error){
        m_status = tristan::network::Status::ERROR;
        m_error = error;
        _notifyWhenError();
        return;
    }
    socket.set_option(asio::ip::tcp::no_delay(true));
    asio::write(socket, asio::buffer(this->prepareRequest()), error);
    if (error){
        m_status = tristan::network::Status::ERROR;
        m_error = error;
        _notifyWhenError();
        return;
    }
    socket.wait(socket.lowest_layer().wait_read);
    std::string response_header;
    while (true){
        std::array<char, 1> character{ };
        asio::read(socket, asio::buffer(character), asio::transfer_exactly(1), error);
        if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
            m_status = tristan::network::Status::ERROR;
            m_error = error;
            _notifyWhenError();
            return;
        }
        response_header += character[0];
        if (character[0] == '\n' && response_header.at(response_header.size() - 3) == '\n'){
            break;
        }
    }
    tristan::network::HttpResponse response(response_header, m_uuid);
    if (response.m_status >= tristan::network::HttpStatus::IM_Used){
        m_response = std::make_shared<tristan::network::HttpResponse>(std::move(response));
        _notifyWhenFinished();
        return;
    }
    _read(socket, response, error);

    socket.lowest_layer().cancel();
    socket.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.lowest_layer().close();
    m_status = tristan::network::Status::DONE;
    m_response = std::make_shared<tristan::network::HttpResponse>(std::move(response));
    _notifyWhenFinished();
}

void tristan::network::HttpRequest::_doHttpsRequest(){
    asio::io_context context;
    asio::ssl::context ssl_context(asio::ssl::context::sslv23_client);
    ssl_context.set_default_verify_paths();
    ssl_context.set_verify_mode(asio::ssl::verify_peer);
    asio::ip::tcp::resolver resolver(context);
    asio::ip::basic_resolver_results<asio::ip::tcp> resolver_results;
    try{
        resolver_results = resolver.resolve(m_uri.host(), m_uri.port());
    }
    catch (const asio::system_error& error){
        m_status = tristan::network::Status::ERROR;
        m_error = error.code();
        _notifyWhenError();
        return;
    }
    asio::ssl::stream<asio::ip::tcp::socket> socket(context, ssl_context);
    asio::error_code error;
    socket.lowest_layer().connect(resolver_results->endpoint(), error);
    if (error){
        m_status = tristan::network::Status::ERROR;
        m_error = error;
        _notifyWhenError();
        return;
    }
    socket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    socket.set_verify_mode(asio::ssl::verify_peer);
    SSL_set_tlsext_host_name(socket.native_handle(), m_uri.host().c_str());
    socket.handshake(asio::ssl::stream_base::client, error);
    if (error){
        m_status = tristan::network::Status::ERROR;
        m_error = error;
        _notifyWhenError();
        return;
    }
    asio::write(socket, asio::buffer(this->prepareRequest()), error);
    if (error){
        m_status = tristan::network::Status::ERROR;
        m_error = error;
        _notifyWhenError();
        return;
    }
    socket.lowest_layer().wait(socket.lowest_layer().wait_read);
    std::string response_header;
    while (true){
        std::array<char, 1> character{ };
        asio::read(socket, asio::buffer(character), asio::transfer_exactly(1), error);
        if (error && error != asio::error::eof && error != asio::ssl::error::stream_truncated){
            m_status = tristan::network::Status::ERROR;
            m_error = error;
            _notifyWhenError();
            return;
        }
        response_header += character[0];
        if (character[0] == '\n' && response_header.at(response_header.size() - 3) == '\n'){
            break;
        }
    }
    tristan::network::HttpResponse response(response_header, m_uuid);
    if (response.m_status >= tristan::network::HttpStatus::IM_Used){
        m_response = std::make_shared<tristan::network::HttpResponse>(std::move(response));
        _notifyWhenFinished();
        return;
    }

    _read(socket, response, error);
    socket.lowest_layer().cancel();
    socket.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.lowest_layer().close();
    m_status = tristan::network::Status::DONE;
    m_response = std::make_shared<tristan::network::HttpResponse>(std::move(response));
    _notifyWhenFinished();
}

tristan::network::GetRequest::GetRequest(Uri url) :
        HttpRequest(std::move(url)){

}

std::string tristan::network::GetRequest::prepareRequest() const{
    std::string l_request = "GET ";
    l_request += m_uri.path();
    if (!m_params.empty()){
        l_request += '?';
        int paramCount = 0;
        for (const auto& param: m_params){
            if (paramCount > 0){
                l_request += '&';
            }
            l_request += param.first;
            if (!param.second.empty()){
                l_request += '=';
                l_request += param.second;
            }
            ++paramCount;
        }
    }
    if (!m_uri.query().empty()){
        l_request += "&" + m_uri.query();
    }
    l_request += " HTTP/1.1\r\n";
    if (!m_headers.empty()){
        for (const auto& header: m_headers){
            l_request += header.first;
            l_request += ":";
            l_request += header.second;
            l_request += "\r\n";
        }
    }
    l_request += "\r\n\r\n";

    return l_request;
}

tristan::network::PostRequest::PostRequest(Uri url) :
        HttpRequest(std::move(url)){

}

std::string tristan::network::PostRequest::prepareRequest() const{
    std::string l_request = "POST /";
    l_request += m_uri.path();
    l_request += " HTTP/1.1\r\n";
    if (!m_headers.empty()){
        for (const auto& header: m_headers){
            l_request += header.first;
            l_request += ":";
            l_request += header.second;
            l_request += "\r\n";
        }
    }
    l_request += "\r\n";
    if (!m_params.empty()){
        int paramCount = 0;
        for (const auto& param: m_params){
            if (paramCount > 0){
                l_request += '&';
            }
            l_request += param.first;
            l_request += '=';
            l_request += param.second;
            ++paramCount;
        }
    }
    l_request += "\r\n\r\n";

    return l_request;
}

