#include "network_request_handler.hpp"
#include "sync_network_request_handler_impl.hpp"
#include "async_request_handler.hpp"
#include "network_logger.hpp"
#include "http_response.hpp"

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

void tristan::network::NetworkRequestsHandler::setLogger(std::unique_ptr< tristan::log::Log >&& p_log) {
    tristan::network::Logger::setLogger(std::move(p_log));
}

void tristan::network::NetworkRequestsHandler::run() { tristan::network::NetworkRequestsHandler::instance()._run(); }

void tristan::network::NetworkRequestsHandler::stop() { tristan::network::NetworkRequestsHandler::instance()._stop(); }

void tristan::network::NetworkRequestsHandler::pause() { tristan::network::NetworkRequestsHandler::instance()._pause(); }

void tristan::network::NetworkRequestsHandler::resume() { tristan::network::NetworkRequestsHandler::instance()._resume(); }

void tristan::network::NetworkRequestsHandler::setActiveDownloadsLimit(uint8_t p_limit) {
    NetworkRequestsHandler::instance().m_async_tcp_requests_handler->setMaxDownloadsCount(p_limit);
}

void tristan::network::NetworkRequestsHandler::notifyWhenExit(std::function< void() >&& p_function) {
    tristan::network::NetworkRequestsHandler::instance()._notifyWhenExit(std::move(p_function));
}

void tristan::network::NetworkRequestsHandler::addRequest(std::shared_ptr< NetworkRequestBase >&& p_request) {
    NetworkRequestsHandler::instance()._addRequest(std::move(p_request));
}

auto tristan::network::NetworkRequestsHandler::pendingRequests() -> std::ranges::ref_view< std::multiset< std::shared_ptr< NetworkRequestBase > > > {
    return {tristan::network::NetworkRequestsHandler::instance().m_requests};
}

auto tristan::network::NetworkRequestsHandler::activeRequests() -> std::list< std::shared_ptr< NetworkRequestBase > >& {
    return tristan::network::NetworkRequestsHandler::instance().m_active_requests;
}

auto tristan::network::NetworkRequestsHandler::errorRequests() -> const std::list< std::shared_ptr< NetworkRequestBase > >& {
    return tristan::network::NetworkRequestsHandler::instance().m_error_requests;
}

void tristan::network::NetworkRequestsHandler::_run() {

    m_async_tcp_requests_handler = tristan::network::private_::AsyncRequestHandler::create();
    m_request_handler = std::make_unique< tristan::network::private_::SyncNetworkRequestHandlerImpl >();
    netInfo("Launching Async request handler");
    m_async_request_handler_thread = std::thread(&tristan::network::private_::AsyncRequestHandler::run, std::ref(*m_async_tcp_requests_handler));
    if (not m_working.load(std::memory_order_relaxed)) {
        m_working.store(true, std::memory_order_relaxed);
    } else {
        netWarning("Function run() was invoked twice");
    }
    while (m_working.load(std::memory_order_relaxed)) {
        if (m_requests.empty() || m_paused.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else {
            std::unique_lock< std::mutex > lock(m_nr_multiset_lock);
            auto network_request = *m_requests.begin();
            m_requests.erase(m_requests.begin());
            lock.unlock();
            auto network_request_uuid = network_request->uuid();
            network_request->addFinishedCallback([this, network_request_uuid]() -> void {
                std::scoped_lock< std::mutex > lock(m_active_nr_lock);
                m_active_requests.remove_if([network_request_uuid](const std::shared_ptr< NetworkRequestBase >& stored_request) {
                    auto stored_request_uuid = stored_request->uuid();
                    netDebug("Removing network request from active requests uuid = " + network_request_uuid);
                    return stored_request_uuid == network_request_uuid;
                });
            });
            network_request->addFailedCallback([this, network_request_uuid]() -> void {
                for (auto iter = m_active_requests.begin(); iter != m_active_requests.end(); ++iter) {
                    if ((*iter)->uuid() == network_request_uuid) {
                        std::scoped_lock< std::mutex > error_lock(m_error_nr_lock);
                        netDebug("Storing network request to failed requests uuid = " + network_request_uuid);
                        m_error_requests.emplace_back(*iter);
                        std::scoped_lock< std::mutex > active_lock(m_active_nr_lock);
                        netDebug("Removing network request from active requests uuid = " + network_request_uuid);
                        m_active_requests.erase(iter);
                        break;
                    }
                }
            });
            std::scoped_lock< std::mutex > active_lock(m_active_nr_lock);
            m_active_requests.emplace_back(network_request);
            if (network_request->priority() == tristan::network::Priority::OUT_OF_QUEUE) {
                m_request_handler->handleRequest(std::move(network_request));
            } else {
                m_async_tcp_requests_handler->addRequest(std::move(network_request));
            }
        }
    }
    if (not m_working.load(std::memory_order_relaxed)) {
        if (not m_notify_when_exit_functions.empty()) {
            for (const auto& function: m_notify_when_exit_functions) {
                function();
            }
        }
    }
}

void tristan::network::NetworkRequestsHandler::_pause() { m_paused.store(true, std::memory_order_relaxed); }

void tristan::network::NetworkRequestsHandler::_resume() { m_paused.store(false, std::memory_order_relaxed); }

void tristan::network::NetworkRequestsHandler::_stop() {
    m_async_tcp_requests_handler->stop();
    m_async_request_handler_thread.join();
    std::scoped_lock< std::mutex > active_lock(m_active_nr_lock);
    std::scoped_lock< std::mutex > nr_lock(m_nr_multiset_lock);
    netInfo("Cancelling active requests");
    for (auto request_iter = m_active_requests.begin(); request_iter != m_active_requests.end();) {
        request_iter->get()->cancel();
        m_requests.insert(*request_iter);
        request_iter = m_active_requests.erase(request_iter);
    }
    m_working.store(false, std::memory_order_relaxed);
}

void tristan::network::NetworkRequestsHandler::_addRequest(std::shared_ptr< NetworkRequestBase >&& request) {
    std::scoped_lock< std::mutex > lock(m_nr_multiset_lock);
    m_requests.emplace(std::move(request));
}

void tristan::network::NetworkRequestsHandler::_notifyWhenExit(std::function< void() >&& p_function) { m_notify_when_exit_functions.emplace_back(p_function); }
