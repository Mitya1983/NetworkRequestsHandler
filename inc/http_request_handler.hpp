#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include "http_request.hpp"

#include <queue>
#include <list>
#include <memory>
#include <utility>
#include <variant>
#include <mutex>
#include <thread>
#include <atomic>

namespace tristan::network{
    class HttpRequestsHandler{
        HttpRequestsHandler();
        static auto instance() -> HttpRequestsHandler&;
      public:
        HttpRequestsHandler(const HttpRequestsHandler& other) = delete;
        HttpRequestsHandler(HttpRequestsHandler&& other) = delete;

        HttpRequestsHandler& operator=(const HttpRequestsHandler& other) = delete;
        HttpRequestsHandler& operator=(HttpRequestsHandler&& other) = delete;

        ~HttpRequestsHandler() = default;

        inline static void setActiveRequestsLimit(uint8_t limit){ HttpRequestsHandler::instance()._setActiveRequestsLimit(limit); }

        inline static void run(){ HttpRequestsHandler::instance()._run(); }

        inline static void stop(){ HttpRequestsHandler::instance()._stop(); }

        template<class Object> inline static void notifyWhenExit(std::weak_ptr<Object> object, void (Object::*functor)()){ HttpRequestsHandler::instance()._notifyWhenExit(object, functor); }

        template<class Object> inline static void notifyWhenExit(Object* object, void (Object::*functor)()){ HttpRequestsHandler::instance()._notifyWhenExit(object, functor); }

        inline static void notifyWhenExit(std::function<void()> functor){ HttpRequestsHandler::instance()._notifyWhenExit(functor); }

        inline static void addRequest(std::shared_ptr<HttpRequest> request){ HttpRequestsHandler::instance()._addRequest(std::move(request)); }

        inline static auto activeRequests() -> std::list<std::shared_ptr<HttpRequest>>{ return HttpRequestsHandler::instance()._activeRequests(); }

      protected:

      private:
        std::mutex m_lock;

        std::queue<std::shared_ptr<HttpRequest>> m_high_priority_requests;
        std::queue<std::shared_ptr<HttpRequest>> m_normal_priority_requests;
        std::queue<std::shared_ptr<HttpRequest>> m_low_priority_requests;
        std::list<std::shared_ptr<HttpRequest>> m_active_requests;

        std::vector<std::function<void()>> m_notify_when_exit_functors;

        uint8_t m_active_requests_limit;
        std::atomic<bool> m_working;

        void _setActiveRequestsLimit(uint8_t limit){ m_active_requests_limit = limit; }

        void _run();
        void _stop();

        void _addRequest(std::shared_ptr<HttpRequest> request);

        auto _activeRequests() -> std::list<std::shared_ptr<HttpRequest>>;

        template<class Object> void _notifyWhenExit(std::weak_ptr<Object> object, void (Object::*functor)()){
            m_notify_when_exit_functors.emplace_back([object, functor]() -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object);
                }
            });
        }

        template<class Object> void _notifyWhenExit(Object* object, void (Object::*functor)()){
            m_notify_when_exit_functors.emplace_back([object, functor]() -> void{
                std::invoke(functor, object);
            });
        }

        void _notifyWhenExit(std::function<void()> functor){
            m_notify_when_exit_functors.emplace_back(functor);
        }
    };

} //End of tristan::network namespace

#endif //HTTP_REQUEST_HANDLER_HPP
