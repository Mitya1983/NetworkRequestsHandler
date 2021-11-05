#ifndef NETWORK_REQUEST_HPP
#define NETWORK_REQUEST_HPP

#include "uri.hpp"
#include "network_utility.hpp"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace tristan::network{
    enum class Status : uint8_t{
        WAITING, PROCESSED, DONE, REDIRECT, ERROR
    };
    enum class Priority : uint8_t{
        LOW, NORMAL, HIGH
    };

    template<class Response> class NetworkRequest{
      public:

        NetworkRequest(const NetworkRequest& other) = delete;
        NetworkRequest(NetworkRequest&& other) noexcept = delete;

        NetworkRequest& operator=(const NetworkRequest& other) = delete;
        NetworkRequest& operator=(NetworkRequest&& other) noexcept = delete;

        virtual ~NetworkRequest() = default;

        template<class Object> void notifyWhenBytesReadChanged(std::weak_ptr<Object> object, void (Object::*functor)(uint64_t)
        ){
            m_notify_read_bytes_changed_functors.emplace_back([object, functor](uint64_t value) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, value);
                }
            });
        }

        template<class Object> void notifyWhenBytesReadChanged(Object* object, void (Object::*functor)(uint64_t)
        ){
            m_notify_read_bytes_changed_functors.emplace_back([object, functor](uint64_t value) -> void{
                std::invoke(functor, object, value);
            });
        }

        template<class Object> void notifyWhenFinished(std::weak_ptr<Object> object, void (Object::*functor)(std::shared_ptr<Response>)
        ){
            m_notify_when_finished_functors.emplace_back([object, functor](std::shared_ptr<Response> request) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, request);
                }
            });
        }

        template<class Object> void notifyWhenFinished(Object* object, void (Object::*functor)(std::shared_ptr<Response>)
        ){
            m_notify_when_finished_functors.emplace_back([object, functor](std::shared_ptr<Response> request) -> void{
                std::invoke(functor, object, request);
            });
        }

        template<class Object> void notifyWhenFinished(std::weak_ptr<Object> object, void (Object::*functor)()
        ){
            m_notify_when_finished_void_functors.emplace_back([object, functor]() -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object);
                }
            });
        }

        template<class Object> void notifyWhenFinished(Object* object, void (Object::*functor)()){
            m_notify_when_finished_void_functors.emplace_back([object, functor]() -> void{ std::invoke(functor, object); });
        }

        void notifyWhenFinished(const std::function<void(std::shared_ptr<Response>)>& functor){
            m_notify_when_finished_functors.emplace_back(functor);
        }

        template<class Object> void notifyWhenPaused(std::weak_ptr<Object> object, void (Object::*functor)(std::shared_ptr<Response>)
        ){
            m_notify_when_paused_functors.emplace_back([object, functor](std::shared_ptr<Response> request) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, request);
                }
            });
        }

        template<class Object> void notifyWhenPaused(Object* object, void (Object::*functor)(std::shared_ptr<Response>)
        ){
            m_notify_when_paused_functors.emplace_back([object, functor](std::shared_ptr<Response> request) -> void{
                std::invoke(functor, object, request);
            });
        }

        template<class Object> void notifyWhenPaused(std::weak_ptr<Object> object, void (Object::*functor)()
        ){
            m_notify_when_paused_void_functors.emplace_back([object, functor]() -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object);
                }
            });
        }

        void notifyWhenPaused(std::function<void()> functor){
            m_notify_when_paused_void_functors.emplace_back(functor);
        }

        template<class Object> void notifyWhenPaused(Object* object, void (Object::*functor)()){
            m_notify_when_paused_void_functors.emplace_back([object, functor]() -> void{ std::invoke(functor, object); });
        }

        template<class Object> void notifyWhenResumed(std::weak_ptr<Object> object, void (Object::*functor)(std::shared_ptr<Response>)
        ){
            m_notify_when_resumed_functors.emplace_back([object, functor](std::shared_ptr<Response> request) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, request);
                }
            });
        }

        template<class Object> void notifyWhenResumed(Object* object, void (Object::*functor)(std::shared_ptr<Response>)
        ){
            m_notify_when_resumed_functors.emplace_back([object, functor](std::shared_ptr<Response> request) -> void{
                std::invoke(functor, object, request);
            });
        }

        template<class Object> void notifyWhenResumed(std::weak_ptr<Object> object, void (Object::*functor)()
        ){
            m_notify_when_resumed_void_functors.emplace_back([object, functor]() -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object);
                }
            });
        }

        template<class Object> void notifyWhenResumed(Object* object, void (Object::*functor)()){
            m_notify_when_resumed_void_functors.emplace_back([object, functor]() -> void{ std::invoke(functor, object); });
        }

        void notifyWhenResumed(std::function<void()> functor){
            m_notify_when_resumed_void_functors.emplace_back(functor);
        }

        template<class Object> void notifyWhenError(std::weak_ptr<Object> object, void (Object::*functor)(const std::pair<std::string, std::error_code>&)){
            m_notify_when_error_functors.emplace_back([object, functor](const std::error_code& error) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, error);
                }
            });
        }

        template<class Object> void notifyWhenError(Object* object, void (Object::*functor)(const std::pair<std::string, std::error_code>&)){
            m_notify_when_error_functors.emplace_back([object, functor](const std::error_code& error) -> void{
                std::invoke(functor, object, error);
            });
        }

        void notifyWhenError(std::function<void(std::pair<std::string, std::error_code>)> functor){
            m_notify_when_error_functors.emplace_back(functor);
        }

        void setPriority(Priority priority){ m_priority = priority; }

        void cancel(){ m_cancelled.store(true); }

        void pauseProcessing(){ m_paused.store(true); }

        void continueProcessing(){ m_paused.store(false); }

        virtual void doRequest(){}

        [[nodiscard]] auto uuid() const noexcept -> const std::string&{ return m_uuid; }

        [[nodiscard]] auto error() const noexcept -> const std::error_code&{ return m_error; }

        [[nodiscard]] auto status() const noexcept -> Status{ return m_status; }

        [[nodiscard]] auto paused() const noexcept -> bool{ return m_paused.load(); }

        [[nodiscard]] auto response() -> std::shared_ptr<Response>{
            if (m_status != tristan::network::Status::DONE){
                this->doRequest();
            }
            return m_response;
        }

        [[nodiscard]] auto priority() -> Priority{ return m_priority; }

        [[nodiscard]] virtual auto prepareRequest() const -> std::string{ return { }; }

      protected:
        explicit NetworkRequest(Uri uri) :
                m_uri(std::move(uri)),
                m_uuid(utility::getUUID()),
                m_bytes_to_read(0),
                m_bytes_read(0),
                m_status(Status::WAITING),
                m_priority(Priority::NORMAL),
                m_output_to_file(false),
                m_paused(false),
                m_cancelled(false){}

        NetworkRequest(const std::string& ip, uint16_t port) :
                m_uuid(utility::getUUID()),
                m_bytes_to_read(0),
                m_bytes_read(0),
                m_status(Status::WAITING),
                m_priority(Priority::NORMAL),
                m_output_to_file(false),
                m_paused(false),
                m_cancelled(false){
            m_uri.setAuthority(ip);
            m_uri.setPort(port);
        }

        Uri m_uri;
        std::error_code m_error;
        std::filesystem::path m_output_path;
        std::string m_uuid;
        std::vector<std::function<void(uint64_t)>> m_notify_read_bytes_changed_functors;
        std::vector<std::function<void(std::shared_ptr<Response>)>> m_notify_when_finished_functors;
        std::vector<std::function<void(std::shared_ptr<Response>)>> m_notify_when_paused_functors;
        std::vector<std::function<void(void)>> m_notify_when_paused_void_functors;
        std::vector<std::function<void(std::shared_ptr<Response>)>> m_notify_when_resumed_functors;
        std::vector<std::function<void(void)>> m_notify_when_resumed_void_functors;
        std::vector<std::function<void(void)>> m_notify_when_finished_void_functors;
        std::vector<std::function<void(std::pair<std::string, std::error_code>)>> m_notify_when_error_functors;

        std::ofstream m_output_file;

        std::shared_ptr<Response> m_response;
        uint64_t m_bytes_to_read;
        uint64_t m_bytes_read;

        const uint16_t m_read_frame = std::numeric_limits<uint16_t>::max();

        Status m_status;
        Priority m_priority;
        bool m_output_to_file;
        std::atomic<bool> m_paused;
        std::atomic<bool> m_cancelled;

        void _notifyWhenBytesReadChanged(){
            if (m_notify_read_bytes_changed_functors.empty()){
                return;
            }
            for (const auto& functor: m_notify_read_bytes_changed_functors){
                functor(m_bytes_read);
            }
        }

        void _notifyWhenFinished(){
            if (m_notify_when_finished_functors.empty() && m_notify_when_finished_void_functors.empty()){
                return;
            }
            for (const auto& functor: m_notify_when_finished_functors){
                functor(m_response);
            }
            for (const auto& functor: m_notify_when_finished_void_functors){
                functor();
            }
        }

        void _notifyWhenPaused(){
            if (m_notify_when_paused_functors.empty() && m_notify_when_paused_void_functors.empty()){
                return;
            }
            for (const auto& functor: m_notify_when_paused_functors){
                functor();
            }
            for (const auto& functor: m_notify_when_paused_void_functors){
                functor();
            }
        }

        void _notifyWhenResumed(){
            if (m_notify_when_resumed_functors.empty() && m_notify_when_resumed_void_functors.empty()){
                return;
            }
            for (const auto& functor: m_notify_when_resumed_functors){
                functor();
            }
            for (const auto& functor: m_notify_when_resumed_void_functors){
                functor();
            }
        }

        void _notifyWhenError(){
            if (m_notify_when_error_functors.empty()){
                return;
            }
            for (const auto& functor: m_notify_when_error_functors){
                functor({ m_uuid,
                          m_error
                        });
            }
        }
    };

} // namespace tristan::network

#endif // NETWORK_REQUEST_HPP
