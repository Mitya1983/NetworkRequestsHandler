#include "http_request_handler.hpp"


//TODO: Add logic to pause lower priority requests if for higher priority request no place to add due request limit
tristan::network::HttpRequestsHandler::HttpRequestsHandler() :
        m_active_requests_limit(5),
        m_active_requests(0),
        m_working(true),
        m_paused(false){

}

auto tristan::network::HttpRequestsHandler::instance() -> tristan::network::HttpRequestsHandler&{
    static tristan::network::HttpRequestsHandler handler;
    return handler;
}

void tristan::network::HttpRequestsHandler::_run(){
    if (m_working.load()){
        return;
    }
    while (m_working.load()){
        if ((m_low_priority_requests.empty() && m_normal_priority_requests.empty() && m_high_priority_requests.empty()) || m_active_requests.size() >= m_active_requests_limit || m_paused){
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        else{
            std::shared_ptr<tristan::network::HttpRequest> request;
            if (!m_high_priority_requests.empty()){
                std::lock_guard<std::mutex> lock(m_lock);
                request = m_high_priority_requests.front();
                m_high_priority_requests.pop();
            }
            else if (!m_normal_priority_requests.empty()){
                std::lock_guard<std::mutex> lock(m_lock);
                request = m_normal_priority_requests.front();
                m_normal_priority_requests.pop();
            }
            else if (!m_low_priority_requests.empty()){
                std::lock_guard<std::mutex> lock(m_lock);
                request = m_high_priority_requests.front();
                m_low_priority_requests.pop();
            }
            request->notifyWhenFinished([this](std::shared_ptr<HttpResponse> response) -> void{
                //TODO: Add lock here
                m_active_requests.remove_if([response](std::shared_ptr<tristan::network::HttpRequest> stored_request){
                    return stored_request->uuid() == response->uuid();
                });
            });
            request->notifyWhenError([this](const std::pair<std::string, std::error_code>& error){
                //TODO: Add lock here
                //TODO: Move request to error queue
                m_active_requests.remove_if([error](std::shared_ptr<tristan::network::HttpRequest> stored_request){
                    return stored_request->uuid() == error.first;
                });
            });
            request->notifyWhenPaused([this]() -> void{
                ++m_active_requests_limit;
            });
            request->notifyWhenResumed([this]() -> void{
                --m_active_requests_limit;
            });
            std::thread(&tristan::network::HttpRequest::doRequest, request).detach();
            m_active_requests.push_back(request);
        }
    }
    if (!m_working.load()){
        if (!m_notify_when_exit_functors.empty()){
            for (const auto& functor: m_notify_when_exit_functors){
                functor();
            }
        }
    }
}

void tristan::network::HttpRequestsHandler::_stop(){
    m_working.store(false);
}

void tristan::network::HttpRequestsHandler::_addRequest(std::shared_ptr<HttpRequest> request){
    std::lock_guard<std::mutex> lock(m_lock);
    switch (request->priority()){
        case Priority::LOW:{
            m_low_priority_requests.push(std::move(request));
            break;
        }
        case Priority::NORMAL:{
            m_normal_priority_requests.push(std::move(request));
            break;
        }
        case Priority::HIGH:{
            m_high_priority_requests.push(std::move(request));
            break;
        }
    }
}

