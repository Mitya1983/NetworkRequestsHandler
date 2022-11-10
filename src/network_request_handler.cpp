#include "network_request_handler.hpp"
#include "net_log.hpp"

#include "asio/connect.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read.hpp"
#include "asio/ssl/error.hpp"
#include "asio/ssl/stream.hpp"

namespace {

    constexpr uint16_t g_max_frame = std::numeric_limits< uint16_t >::max();
    /**
     * In thread we read up to 4Mb. Everything else goes to downloader.
     */
    constexpr uint32_t g_thread_download_limit = g_max_frame * 40;
    template < class Socket >
    void read_data(Socket& socket,
                   std::shared_ptr< tristan::network::NetworkRequest > network_request,
                   std::error_code& error_code);
}  // End of anonymous namespace

// void tristan::network::NetworkRequestsHandler::_run(){
//     if (m_working.load(std::memory_order_relaxed)){
//         return;
//     }
//     while (m_working.load(std::memory_order_relaxed)){
//         if ((m_low_priority_requests.empty() &&
//         m_normal_priority_requests.empty() &&
//         m_high_priority_requests.empty()) || m_active_requests.size() >=
//         m_active_requests_limit || m_paused){
//             std::this_thread::sleep_for(std::chrono::milliseconds(500));
//         }
//         else{
//             std::shared_ptr<tristan::network::HttpRequest> request;
//             if (!m_high_priority_requests.empty()){
//                 std::scoped_lock<std::mutex> lock(m_hp_lock);
//                 request = m_high_priority_requests.front();
//                 m_high_priority_requests.pop();
//             }
//             else if (!m_normal_priority_requests.empty()){
//                 std::scoped_lock<std::mutex> lock(m_np_lock);
//                 request = m_normal_priority_requests.front();
//                 m_normal_priority_requests.pop();
//             }
//             else if (!m_low_priority_requests.empty()){
//                 std::scoped_lock<std::mutex> lock(m_lp_lock);
//                 request = m_high_priority_requests.front();
//                 m_low_priority_requests.pop();
//             }
//             request->notifyWhenFinished([this](std::shared_ptr<HttpResponse>
//             response) -> void{
//                 std::scoped_lock<std::mutex> lock(m_active_lock);
//                 m_active_requests.remove_if([response](std::shared_ptr<tristan::network::HttpRequest>
//                 stored_request){
//                     return stored_request->uuid() == response->uuid();
//                 });
//             });
//             request->notifyWhenError([this](const std::pair<std::string,
//             std::error_code>& error){
//                 for (auto&& l_request : m_active_requests){
//                     if (l_request->uuid() == error.first){
//                         std::scoped_lock<std::mutex> lock(m_error_lock);
//                         m_error_requests.push(l_request);
//                     }
//                 }
//                 m_active_requests.remove_if([error](std::shared_ptr<tristan::network::HttpRequest>
//                 stored_request){
//                     return stored_request->uuid() == error.first;
//                 });
//             });
//             request->notifyWhenPaused([this]() -> void{
//                 ++m_active_requests_limit;
//             });
//             request->notifyWhenResumed([this]() -> void{
//                 --m_active_requests_limit;
//             });
//             std::thread(&tristan::network::HttpRequest::doRequest,
//             request).detach(); std::scoped_lock<std::mutex>
//             lock(m_active_lock); m_active_requests.push_back(request);
//         }
//     }
//     if (!m_working.load(std::memory_order_relaxed)){
//         if (!m_notify_when_exit_functors.empty()){
//             for (const auto& functor: m_notify_when_exit_functors){
//                 functor();
//             }
//         }
//     }
// }

// void tristan::network::NetworkRequestsHandler::_stop(){
//     m_working.store(false, std::memory_order_relaxed);
// }

// void
// tristan::network::NetworkRequestsHandler::_addRequest(std::shared_ptr<HttpRequest>
// request){
//
//     switch (request->priority()){
//         case Priority::LOW:{
//             std::scoped_lock<std::mutex> lock(m_lp_lock);
//             m_low_priority_requests.push(std::move(request));
//             break;
//         }
//         case Priority::NORMAL:{
//             std::scoped_lock<std::mutex> lock(m_np_lock);
//             m_normal_priority_requests.push(std::move(request));
//             break;
//         }
//         case Priority::HIGH:{
//             std::scoped_lock<std::mutex> lock(m_hp_lock);
//             m_high_priority_requests.push(std::move(request));
//             if (m_active_requests.size() >= m_active_requests_limit){
//                 for (const auto& l_request : m_active_requests){
//                     if (l_request->priority() == Priority::LOW){
//                         l_request->pauseProcessing();
//                     }
//                 }
//             }
//             break;
//         }
//     }
// }

tristan::network::NetworkRequestsHandler::NetworkRequestsHandler() :
    m_downloader(tristan::network::AsyncTcpRequestHandler::create()) { }

auto tristan::network::NetworkRequestsHandler::instance() -> tristan::network::NetworkRequestsHandler& {
    static NetworkRequestsHandler network_requests_handler;

    return network_requests_handler;
}

void tristan::network::NetworkRequestsHandler::_run() {
    if (m_working.load(std::memory_order_relaxed)) {
        return;
    }
    while (m_working.load(std::memory_order_relaxed)) {
        if (m_requests.empty() || m_paused.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else {
            std::unique_lock< std::mutex > lock(m_nr_queue_lock);
            auto request = m_requests.top();
            m_requests.pop();
            lock.unlock();
            std::visit(
                [this](const auto& value) -> void {
                    auto request_uuid = value->uuid();
                    value->addFinishedCallback([this, request_uuid]() -> void {
                        std::scoped_lock< std::mutex > lock(m_active_nr_lock);
                        m_active_requests.remove_if(
                            [request_uuid](const SuppoertedRequestTypes& stored_request) {
                                auto stored_request_uuid = std::visit(
                                    [](const auto& value) {
                                        return value->uuid();
                                    },
                                    stored_request);
                                return stored_request_uuid == request_uuid;
                            });
                    });
                    value->addFailedCallback([this, request_uuid]() -> void {
                        for (auto l_request: m_active_requests) {
                            auto failed_network_request_uuid = std::visit(
                                [](auto active_request) {
                                    return active_request->uuid();
                                },
                                l_request);
                            if (failed_network_request_uuid == request_uuid) {
                                std::scoped_lock< std::mutex > lock(m_error_nr_lock);
                                m_error_requests.emplace_back(l_request);
                                m_active_requests.remove_if(
                                    [request_uuid](const SuppoertedRequestTypes& stored_request) {
                                        auto stored_request_uuid = std::visit(
                                            [](const auto& value) {
                                                return value->uuid();
                                            },
                                            stored_request);
                                        return stored_request_uuid == request_uuid;
                                    });
                                break;
                            }
                        }
                    });
                },
                request);
            std::scoped_lock< std::mutex > active_lock(m_active_nr_lock);
            m_active_requests.emplace_back(request);
            std::visit(
                [this](auto value) -> void {
                    using T = std::decay_t< decltype(value)>;
                    if constexpr (std::is_same_v<T, std::shared_ptr<NetworkRequest>>) {
                        std::thread(&NetworkRequestsHandler::_processTcpRequest, this, value);
                    }
                },
                request);
        }
    }
    if (not m_working.load(std::memory_order_relaxed)) {
        if (not m_notify_when_exit_functors.empty()) {
            for (const auto& functor: m_notify_when_exit_functors) {
                functor();
            }
        }
    }
}

void tristan::network::NetworkRequestsHandler::_processTcpRequest(
    std::shared_ptr< tristan::network::NetworkRequest > tcp_request) {

    netTrace("Start");
    netDebug("Processing TCP request " + tcp_request->uuid());
    asio::io_context context;
    if (tcp_request->url().hostIP().empty()) {
        netInfo("Request IP is empty. Resolving from host name");
        if (tcp_request->url().host().empty()) {
            netError("Request host is empty");
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(
                tcp_request, tristan::network::makeError(tristan::network::ErrorCode::INVALID_URL));
            netTrace("End");
            return;
        }
        asio::ip::tcp::resolver resolver(context);
        asio::ip::basic_resolver_results< asio::ip::tcp > resolver_results;
        try {
            resolver_results = resolver.resolve(tcp_request->url().host(), tcp_request->url().port());
            const_cast< tristan::network::Url& >(tcp_request->url())
                .setHostIP(resolver_results->endpoint().address().to_string());
            netInfo("Address was resolved to " + tcp_request->url().hostIP());
        } catch (const asio::system_error& error) {
            netError("Resolver: " + error.code().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(tcp_request, error.code());
            netTrace("End");
            return;
        }
    }
    asio::error_code error_code;
    if (not tcp_request->isSSL()) {
        asio::ip::tcp::socket socket(context);
        netDebug("Connecting to address " + tcp_request->url().hostIP() + " " + tcp_request->url().port());
        socket.lowest_layer().connect(
            asio::ip::tcp::endpoint(asio::ip::address::from_string(tcp_request->url().hostIP()),
                                    tcp_request->url().portUint16_t()),
            error_code);
        if (error_code) {
            netError("Asio connect: " + error_code.message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(tcp_request, error_code);
            netTrace("End");
            return;
        }
        socket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
        netDebug("Writing to address " + tcp_request->url().hostIP() + " a message "
                 + std::string(tcp_request->requestData().begin(), tcp_request->requestData().end()));
        asio::write(socket, asio::buffer(tcp_request->requestData()), error_code);
        if (error_code) {
            netError("Asio write: " + error_code.message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(tcp_request, error_code);
            netTrace("End");
            return;
        }
        socket.lowest_layer().wait(socket.lowest_layer().wait_read);
        if (tcp_request->bytesToRead() != 0 && tcp_request->bytesToRead() > g_thread_download_limit) {
            netInfo("Request size exceeds 4Mb. Sending to downloader");
            m_downloader->addDownload(socket, std::move(tcp_request));
        } else {
            read_data(socket, tcp_request, error_code);
        }
    } else {
        asio::ssl::context ssl_context(asio::ssl::context::sslv23_client);
        ssl_context.set_default_verify_paths();
        ssl_context.set_verify_mode(asio::ssl::verify_peer);
        asio::ssl::stream< asio::ip::tcp::socket > socket(context, ssl_context);
    }

    netTrace("End");
}

namespace {

    template < class Socket >
    void read_data(Socket& socket,
                   std::shared_ptr< tristan::network::NetworkRequest > network_request,
                   std::error_code& error_code) {

        if (network_request->bytesToRead() != 0) {
            netDebug("Reading by size");
            uint64_t bytes_read = 0;
            uint64_t bytes_to_read = network_request->bytesToRead();
            while (bytes_read < bytes_to_read) {
                std::vector< uint8_t > buf;
                auto bytes_remain = bytes_to_read - bytes_read;
                uint16_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);
                buf.reserve(current_frame_size);

                asio::read(socket,
                           asio::dynamic_buffer(buf),
                           asio::transfer_exactly(current_frame_size),
                           error_code);
                if (error_code && error_code != asio::error::eof
                    && error_code != asio::ssl::error::stream_truncated) {
                    tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                                  error_code);
                    break;
                }
                bytes_read += current_frame_size;
                tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request,
                                                                                     std::move(buf));
                tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(
                    network_request, tristan::network::Status::DONE);
            }
        } else if (not network_request->responseDelimiter().empty()) {
            netDebug("Reading by delimiter");
            const auto& delimiter = network_request->responseDelimiter();
            const auto delimiter_size = delimiter.size();
            std::vector< uint8_t > buf;
            buf.reserve(g_max_frame);
            while (true) {
                std::vector< uint8_t > bytes_to_read(delimiter_size);
                asio::read(socket,
                           asio::dynamic_buffer(bytes_to_read),
                           asio::transfer_exactly(delimiter_size),
                           error_code);
                if (error_code && error_code != asio::error::eof
                    && error_code != asio::ssl::error::stream_truncated) {
                    tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                                  error_code);
                    break;
                }
                if (bytes_to_read == delimiter) {
                    netInfo("Delimiter reached");
                    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(
                        network_request, tristan::network::Status::DONE);
                    break;
                }
                buf.insert(buf.end(), bytes_to_read.begin(), bytes_to_read.end());
                if (buf.size() == g_max_frame) {
                    tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request,
                                                                                         std::move(buf));
                    buf.reserve(g_max_frame);  //NOLINT
                }
            }
        } else {
            error_code
                = tristan::network::makeError(tristan::network::ErrorCode::REQUEST_SIZE_IS_NOT_APPROPRIATE);
            netError(error_code.message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, error_code);
            netTrace("End");
            return;
        }
    }
}  // End of anonymous namespace
