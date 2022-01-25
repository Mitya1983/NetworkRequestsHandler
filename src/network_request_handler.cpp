//#include "network_request_handler.hpp"

//void tristan::network::NetworkRequestsHandler::_run(){
//    if (m_working.load(std::memory_order_relaxed)){
//        return;
//    }
//    while (m_working.load(std::memory_order_relaxed)){
//        if ((m_low_priority_requests.empty() && m_normal_priority_requests.empty() && m_high_priority_requests.empty()) || m_active_requests.size() >= m_active_requests_limit || m_paused){
//            std::this_thread::sleep_for(std::chrono::milliseconds(500));
//        }
//        else{
//            std::shared_ptr<tristan::network::HttpRequest> request;
//            if (!m_high_priority_requests.empty()){
//                std::scoped_lock<std::mutex> lock(m_hp_lock);
//                request = m_high_priority_requests.front();
//                m_high_priority_requests.pop();
//            }
//            else if (!m_normal_priority_requests.empty()){
//                std::scoped_lock<std::mutex> lock(m_np_lock);
//                request = m_normal_priority_requests.front();
//                m_normal_priority_requests.pop();
//            }
//            else if (!m_low_priority_requests.empty()){
//                std::scoped_lock<std::mutex> lock(m_lp_lock);
//                request = m_high_priority_requests.front();
//                m_low_priority_requests.pop();
//            }
//            request->notifyWhenFinished([this](std::shared_ptr<HttpResponse> response) -> void{
//                std::scoped_lock<std::mutex> lock(m_active_lock);
//                m_active_requests.remove_if([response](std::shared_ptr<tristan::network::HttpRequest> stored_request){
//                    return stored_request->uuid() == response->uuid();
//                });
//            });
//            request->notifyWhenError([this](const std::pair<std::string, std::error_code>& error){
//                for (auto&& l_request : m_active_requests){
//                    if (l_request->uuid() == error.first){
//                        std::scoped_lock<std::mutex> lock(m_error_lock);
//                        m_error_requests.push(l_request);
//                    }
//                }
//                m_active_requests.remove_if([error](std::shared_ptr<tristan::network::HttpRequest> stored_request){
//                    return stored_request->uuid() == error.first;
//                });
//            });
//            request->notifyWhenPaused([this]() -> void{
//                ++m_active_requests_limit;
//            });
//            request->notifyWhenResumed([this]() -> void{
//                --m_active_requests_limit;
//            });
//            std::thread(&tristan::network::HttpRequest::doRequest, request).detach();
//            std::scoped_lock<std::mutex> lock(m_active_lock);
//            m_active_requests.push_back(request);
//        }
//    }
//    if (!m_working.load(std::memory_order_relaxed)){
//        if (!m_notify_when_exit_functors.empty()){
//            for (const auto& functor: m_notify_when_exit_functors){
//                functor();
//            }
//        }
//    }
//}

//void tristan::network::NetworkRequestsHandler::_stop(){
//    m_working.store(false, std::memory_order_relaxed);
//}

//void tristan::network::NetworkRequestsHandler::_addRequest(std::shared_ptr<HttpRequest> request){
//
//    switch (request->priority()){
//        case Priority::LOW:{
//            std::scoped_lock<std::mutex> lock(m_lp_lock);
//            m_low_priority_requests.push(std::move(request));
//            break;
//        }
//        case Priority::NORMAL:{
//            std::scoped_lock<std::mutex> lock(m_np_lock);
//            m_normal_priority_requests.push(std::move(request));
//            break;
//        }
//        case Priority::HIGH:{
//            std::scoped_lock<std::mutex> lock(m_hp_lock);
//            m_high_priority_requests.push(std::move(request));
//            if (m_active_requests.size() >= m_active_requests_limit){
//                for (const auto& l_request : m_active_requests){
//                    if (l_request->priority() == Priority::LOW){
//                        l_request->pauseProcessing();
//                    }
//                }
//            }
//            break;
//        }
//    }
//}

