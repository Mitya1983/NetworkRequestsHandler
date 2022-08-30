#include "network_request_handler.hpp"
#include "net_log.hpp"

#include "asio/connect.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read.hpp"
#include "asio/ssl/error.hpp"
#include "asio/ssl/stream.hpp"

namespace {
    void processTCPRequest(std::shared_ptr< tristan::network::NetworkRequest > request);
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
    m_downloader(tristan::network::Downloader::create()) { }

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
            auto request_uuid = request->uuid();
            request->addFinishedCallback([this, request_uuid]() -> void {
                std::scoped_lock< std::mutex > lock(m_active_nr_lock);
                m_active_requests.remove_if(
                    [request_uuid](
                        const std::shared_ptr< tristan::network::NetworkRequest >& stored_request) {
                        return stored_request->uuid() == request_uuid;
                    });
            });
            request->addFailedCallback([this](const std::string& uuid, std::error_code error_code) {
                for (auto&& l_request: m_active_requests) {
                    if (l_request->uuid() == uuid) {
                        std::scoped_lock< std::mutex > lock(m_error_nr_lock);
                        m_error_requests.push_back(l_request);
                    }
                }
                std::scoped_lock< std::mutex > lock(m_active_nr_lock);
                m_active_requests.remove_if(
                    [uuid](const std::shared_ptr< tristan::network::NetworkRequest >& stored_request) {
                        return stored_request->uuid() == uuid;
                    });
            });
            std::thread(&Request::doRequest, request).detach();
            std::scoped_lock< std::mutex > active_lock(m_active_nr_lock);
            m_active_requests.push_back(request);
        }
    }
    if (!m_working.load(std::memory_order_relaxed)) {
        if (!m_notify_when_exit_functors.empty()) {
            for (const auto& functor: m_notify_when_exit_functors) {
                functor();
            }
        }
    }
}

namespace {
    void processTCPRequest(std::shared_ptr< tristan::network::NetworkRequest > request) {
        netTrace("Start");
        netDebug("Processing TCP request " + request->uuid());
        asio::io_context context;
        if (request->url().hostIP().empty()) {
            netInfo("Request IP is empty. Resolving from host name");
            if (request->url().host().empty()) {
                netError("Request host is empty");
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(
                    request, tristan::network::makeError(tristan::network::ErrorCode::INVALID_URL));
                netTrace("End");
                return;
            }
            asio::ip::tcp::resolver resolver(context);
            asio::ip::basic_resolver_results< asio::ip::tcp > resolver_results;
            try {
                resolver_results = resolver.resolve(request->url().host(), request->url().port());
                const_cast< tristan::network::Url& >(request->url())
                    .setHostIP(resolver_results->endpoint().address().to_string());
                netInfo("Address was resolved to " + request->url().hostIP());
            } catch (const asio::system_error& error) {
                netError("Resolver: " + error.code().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(request, error.code());
                netTrace("End");
                return;
            }
        }
        asio::error_code error;
        if (!request->isSSL()) {
            asio::ssl::context ssl_context(asio::ssl::context::sslv23_client);
            asio::ip::tcp::socket socket(context);

            socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(request->url().hostIP()),
                                                   request->url().portUint16_t()),
                           error);
            if (error) {
                netError("Asio connect: " + error.message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(request, error);
                netTrace("End");
                return;
            }
            socket.set_option(asio::ip::tcp::no_delay(true));
            netDebug("Writing to socket "
                     + std::string(request->requestData().begin(), request->requestData().end()));
            asio::write(socket, asio::buffer(request->requestData()), error);
            if (error) {
                netError("Asio write: " + error.message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(request, error);
                netTrace("End");
                return;
            }
        } else {
        }

        netTrace("End");
    }
}  // End of anonymous namespace
