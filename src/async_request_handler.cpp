#include "async_request_handler.hpp"
#include "async_network_request_handler_impl.hpp"
#include "network_error.hpp"
#include "network_logger.hpp"
#include "http_response.hpp"

#include <resumable_coroutine.hpp>

#include <thread>

tristan::network::private_::AsyncRequestHandler::AsyncRequestHandler() :
    m_max_processed_requests_count(50),
    m_working(false) { }

tristan::network::private_::AsyncRequestHandler::~AsyncRequestHandler() = default;

auto tristan::network::private_::AsyncRequestHandler::create() -> std::unique_ptr< AsyncRequestHandler > {
    return std::unique_ptr< AsyncRequestHandler >(new AsyncRequestHandler());
}

void tristan::network::private_::AsyncRequestHandler::setMaxDownloadsCount(uint8_t p_count) { m_max_processed_requests_count = p_count; }

void tristan::network::private_::AsyncRequestHandler::stop() { m_working.store(false, std::memory_order_relaxed); }

void tristan::network::private_::AsyncRequestHandler::run() {
    netInfo("Starting async request handler");
    if (not m_request_handler){
        m_request_handler = std::make_unique< tristan::network::private_::AsyncNetworkRequestHandlerImpl >();
    }
    m_working.store(true, std::memory_order_relaxed);

    while (m_working) {
        if (m_processed_requests.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else {
            uint8_t active_requests_counter = 0;
            for (auto active_requests_iterator = m_processed_requests.begin(); active_requests_iterator != m_processed_requests.end();) {
                if (active_requests_counter > m_max_processed_requests_count) {
                    continue;
                    netInfo("Maximum async requests number is reached");
                }
                if (not active_requests_iterator->resume()) {
                    std::scoped_lock< std::mutex > lock(m_processed_requests_lock);
                    active_requests_iterator = m_processed_requests.erase(active_requests_iterator);
                    --active_requests_counter;
                } else {
                    ++active_requests_iterator;
                    ++active_requests_counter;
                }
            }
        }
    }
    netInfo("Async request handler stopped");
}

void tristan::network::private_::AsyncRequestHandler::addRequest(std::shared_ptr< NetworkRequestBase >&& p_network_request) {
    if (not m_working) {
        p_network_request->request_handlers_api.setError(tristan::network::makeError(tristan::network::ErrorCode::ASYNC_NETWORK_REQUEST_HANDLER_WAS_NOT_LUNCHED));
        return;
    }
    std::scoped_lock< std::mutex > lock(m_processed_requests_lock);

    m_processed_requests.emplace_back(m_request_handler->handleRequest(std::move(p_network_request)));
}
