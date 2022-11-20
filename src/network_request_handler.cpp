#include "network_request_handler.hpp"
#include "tcp_request_handler.hpp"
#include "net_log.hpp"

namespace {

    constexpr uint8_t g_max_frame = std::numeric_limits< uint8_t >::max();

    //    template < class Socket > void read_data(Socket& socket, std::shared_ptr< tristan::network::NetworkRequest > network_request, std::error_code& error_code);
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
tristan::network::NetworkRequestsHandler::NetworkRequestsHandler() :
    m_time_out_interval(std::chrono::seconds(10)) { }

tristan::network::NetworkRequestsHandler::~NetworkRequestsHandler() {
    if (m_working.load(std::memory_order_relaxed)) {
        netWarning("Loop was not stopped before destructor and all requests being processed will be discarded.");
        tristan::network::NetworkRequestsHandler::_stop();
    }
}

auto tristan::network::NetworkRequestsHandler::instance() -> tristan::network::NetworkRequestsHandler& {
    static NetworkRequestsHandler network_requests_handler;

    return network_requests_handler;
}

void tristan::network::NetworkRequestsHandler::addRequest(tristan::network::SuppoertedRequestTypes&& request) {
    netDebug("debug");
    auto is_derived_from_network_request = std::visit(
        [](const auto& shared_pointer) -> bool {
            using T = std::decay_t< decltype(shared_pointer.get()) >;
            return std::is_base_of_v< NetworkRequest*, T > || std::is_same_v< NetworkRequest*, T >;
        },
        request);
    if (not is_derived_from_network_request) {
        throw std::invalid_argument("Object passed to NetworkRequestHandler is not derived from NetworkRequest");
    }
    NetworkRequestsHandler::instance()._addRequest(std::move(request));
}

void tristan::network::NetworkRequestsHandler::_run() {

    m_async_tcp_requests_handler = tristan::network::AsyncTcpRequestHandler::create();
    netInfo("Launching Async request handler");
    m_async_request_handler_thread = std::thread(&tristan::network::AsyncTcpRequestHandler::run, std::ref(*m_async_tcp_requests_handler));
    if (not m_working.load(std::memory_order_relaxed)) {
        m_working.store(true, std::memory_order_relaxed);
    } else {
        netWarning("Function run() was invoked twice");
    }
    while (m_working.load(std::memory_order_relaxed)) {
        if (m_requests.empty() || m_paused.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else {
            std::unique_lock< std::mutex > lock(m_nr_queue_lock);
            auto request = m_requests.top();
            m_requests.pop();
            netDebug("request.index() = " + std::to_string(request.index()));
            lock.unlock();
            std::visit(
                [this](const auto& value) -> void {
                    auto request_uuid = value->uuid();
                    value->addFinishedCallback([this, request_uuid]() -> void {
                        std::scoped_lock< std::mutex > lock(m_active_nr_lock);
                        m_active_requests.remove_if([request_uuid](const SuppoertedRequestTypes& stored_request) {
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
                                m_active_requests.remove_if([request_uuid](const SuppoertedRequestTypes& stored_request) {
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
                    using T = std::decay_t< decltype(value) >;
                    if constexpr (std::is_same_v< T, std::shared_ptr< NetworkRequest > >) {
                        if (value->priority() == tristan::network::Priority::OUT_OF_QUEUE){
                            std::thread(&tristan::network::TcpRequestHandler::processRequest, tristan::network::TcpRequestHandler(), std::move(value)).detach();
                        } else {
                            m_async_tcp_requests_handler->addRequest(std::move(value));
                        }
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

void tristan::network::NetworkRequestsHandler::_pause() { m_paused.store(true, std::memory_order_relaxed); }

void tristan::network::NetworkRequestsHandler::_resume() { m_paused.store(false, std::memory_order_relaxed); }

void tristan::network::NetworkRequestsHandler::_stop() {
    m_async_tcp_requests_handler->stop();
    m_async_request_handler_thread.join();
    m_working.store(false, std::memory_order_relaxed);
}

void tristan::network::NetworkRequestsHandler::_addRequest(tristan::network::SuppoertedRequestTypes&& request) {
    std::scoped_lock< std::mutex > lock(m_nr_queue_lock);
    m_requests.emplace(std::move(request));
}

void tristan::network::NetworkRequestsHandler::_processRequest(tristan::network::SuppoertedRequestTypes&& request) {
    std::visit(
        [this](auto value) -> void {
            using T = std::decay_t< decltype(value) >;
            if constexpr (std::is_same_v< T, std::shared_ptr< NetworkRequest > >) {
                std::thread(&NetworkRequestsHandler::_processTcpRequest, this, value);
            }
        },
        request);
}

void tristan::network::NetworkRequestsHandler::_processTcpRequest(std::shared_ptr< tristan::network::NetworkRequest > network_request) {

    netTrace("Start");
    netInfo("Starting processing of request " + network_request->uuid());

    netDebug("network_request->uuid() = " + network_request->uuid());
    netDebug("network_request->url().hostIP().as_string = " + network_request->url().hostIP().as_string);
    netDebug("network_request->url().port() = " + network_request->url().port());
    netDebug("network_request->url() = " + network_request->url().composeUrl());
    netDebug("network_request->requestData() = " + std::string(network_request->requestData().begin(), network_request->requestData().end()));
    netDebug("network_request->requestData().size() = " + std::to_string(network_request->requestData().size()));
    netDebug("network_request->bytesToRead() = " + std::to_string(network_request->bytesToRead()));
    netDebug("network_request->responseDelimiter() = " + std::string(network_request->responseDelimiter().begin(), network_request->responseDelimiter().end()));

    tristan::network::Socket socket;

    if (socket.error()) {
        netError(socket.error().message());
        tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
        return;
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::PROCESSED);

    socket.setHost(network_request->url().hostIP().as_int, network_request->url().host());
    socket.setRemotePort(network_request->url().portUint16_t_network_byte_order());
    socket.setNonBlocking();
    std::chrono::microseconds time_out;
    while (not socket.connected()) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            return;
        }
        netInfo("Connecting to " + network_request->url().hostIP().as_string);
        netDebug("network_request->uuid() = " + network_request->uuid());
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        socket.connect();

        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::CONNECT_TRY_AGAIN)) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
            return;
        }
        auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        time_out += end - start;
        if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                          tristan::network::makeError(tristan::network::SocketErrors::CONNECT_TIMED_OUT));
            return;
        }
    }

    uint64_t bytes_written = 0;
    uint64_t bytes_to_write = network_request->requestData().size();

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::WRITING);
    time_out = std::chrono::microseconds(0);
    while (bytes_written < bytes_to_write) {
        if (network_request->isPaused()) {
            netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
            return;
        }
        netInfo("Writing to " + network_request->url().hostIP().as_string);
        auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        auto bytes_remain = bytes_to_write - bytes_written;
        uint8_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);
        bytes_written += socket.write(network_request->requestData(), current_frame_size, bytes_written);
        if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::WRITE_TRY_AGAIN)) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
            return;
        }
        auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
        time_out += end - start;
        if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
            netError(socket.error().message());
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                          tristan::network::makeError(tristan::network::SocketErrors::WRITE_TIMED_OUT));
            return;
        }
    }

    if (network_request->bytesToRead() != 0) {
        uint64_t bytes_read = 0;
        uint64_t bytes_to_read = network_request->bytesToRead();
        while (bytes_read < bytes_to_read) {
            if (network_request->isPaused()) {
                netInfo("Network request is paused network_request->uuid() = " + network_request->uuid());
                return;
            }
            auto start = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            auto bytes_remain = bytes_to_read - bytes_read;
            uint8_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);

            auto data = socket.read(current_frame_size);
            if (socket.error() && socket.error().value() != static_cast< int >(tristan::network::SocketErrors::READ_TRY_AGAIN)) {
                netError(socket.error().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, socket.error());
                return;
            }
            if (not data.empty()) {
                netDebug("data.size() = " + std::to_string(data.size()));
                netDebug("data = " + std::string(data.begin(), data.end()));
                bytes_read += data.size();
                tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request, std::move(data));
            }
            auto end = std::chrono::time_point_cast< std::chrono::microseconds >(std::chrono::system_clock::now());
            time_out += end - start;
            if (std::chrono::duration_cast< std::chrono::seconds >(time_out) >= m_time_out_interval) {
                netError(socket.error().message());
                tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request,
                                                                              tristan::network::makeError(tristan::network::SocketErrors::READ_TIMED_OUT));
                return;
            }
        }
    }

    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::DONE);
    netInfo("Request " + network_request->uuid() + " successfully processed");

    netTrace("End");
}

namespace {

    //    template < class Socket > void read_data(Socket& socket, std::shared_ptr< tristan::network::NetworkRequest > network_request, std::error_code& error_code) {
    //
    //        if (network_request->bytesToRead() != 0) {
    //            netDebug("Reading by size");
    //            uint64_t bytes_read = 0;
    //            uint64_t bytes_to_read = network_request->bytesToRead();
    //            while (bytes_read < bytes_to_read) {
    //                std::vector< uint8_t > buf;
    //                auto bytes_remain = bytes_to_read - bytes_read;
    //                uint16_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);
    //                buf.reserve(current_frame_size);
    //
    //                asio::read(socket, asio::dynamic_buffer(buf), asio::transfer_exactly(current_frame_size), error_code);
    //                if (error_code && error_code != asio::error::eof && error_code != asio::ssl::error::stream_truncated) {
    //                    tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, error_code);
    //                    break;
    //                }
    //                bytes_read += current_frame_size;
    //                tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request, std::move(buf));
    //                tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::DONE);
    //            }
    //        } else if (not network_request->responseDelimiter().empty()) {
    //            netDebug("Reading by delimiter");
    //            const auto& delimiter = network_request->responseDelimiter();
    //            const auto delimiter_size = delimiter.size();
    //            std::vector< uint8_t > buf;
    //            buf.reserve(g_max_frame);
    //            while (true) {
    //                std::vector< uint8_t > bytes_to_read(delimiter_size);
    //                asio::read(socket, asio::dynamic_buffer(bytes_to_read), asio::transfer_exactly(delimiter_size), error_code);
    //                if (error_code && error_code != asio::error::eof && error_code != asio::ssl::error::stream_truncated) {
    //                    tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, error_code);
    //                    break;
    //                }
    //                if (bytes_to_read == delimiter) {
    //                    netInfo("Delimiter reached");
    //                    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request, tristan::network::Status::DONE);
    //                    break;
    //                }
    //                buf.insert(buf.end(), bytes_to_read.begin(), bytes_to_read.end());
    //                if (buf.size() == g_max_frame) {
    //                    tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request, std::move(buf));
    //                    buf.reserve(g_max_frame);  //NOLINT
    //                }
    //            }
    //        } else {
    //            error_code = tristan::network::makeError(tristan::network::ErrorCode::REQUEST_SIZE_IS_NOT_APPROPRIATE);
    //            netError(error_code.message());
    //            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, error_code);
    //            netTrace("End");
    //            return;
    //        }
    //    }
}  // End of anonymous namespace
