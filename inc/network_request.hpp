#ifndef NETWORK_REQUEST_HPP
#define NETWORK_REQUEST_HPP

#include "uri.hpp"
#include "network_utility.hpp"
#include "network_response.hpp"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <concepts>

namespace tristan::network{
    /**
     * \enum Status
     * \brief List of possible request statuses
     */
    enum class Status : uint8_t{
        /// Request is created but not being processed yet
        WAITING, /// Request is being processed by request handler
        PROCESSED, /// Request had been processed
        DONE, /// System error occurred
        ERROR
    };
    /**
     * \enum Priority
     * \brief List of possible priority values
     */
    enum class Priority : uint8_t{
        LOW, NORMAL, HIGH
    };

    /**
     * \class template<class Response> class NetworkRequest
     * \brief Used as a base class for network request classes. E.g. HttpRequest.
     * \tparam Response Type of the network response which will be returned after request is processed.
     */
    template<class Response> requires IsDerivedFromNetworkResponse<Response> class NetworkRequest{

      public:

        friend bool operator<(const NetworkRequest& left, const NetworkRequest& right){
            return left.m_priority < right.m_priority;
        }
        friend bool operator>(const NetworkRequest& left, const NetworkRequest& right){
            return !(left < right);
        }

        NetworkRequest(const NetworkRequest& other) = delete;
        NetworkRequest(NetworkRequest&& other) noexcept = delete;

        NetworkRequest& operator=(const NetworkRequest& other) = delete;
        NetworkRequest& operator=(NetworkRequest&& other) noexcept = delete;

        virtual ~NetworkRequest() = default;

        /**
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object> object.
         * \param functor void (Object::*functor)(uint64_t)
         */
        template<class Object> void notifyWhenBytesReadChanged(std::weak_ptr<Object> object, void (Object::*functor)(uint64_t)){
            m_notify_read_bytes_changed_functors.emplace_back([object, functor](uint64_t value) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, value);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)(uint64_t)
         */
        template<class Object> void notifyWhenBytesReadChanged(Object* object, void (Object::*functor)(uint64_t)){
            m_notify_read_bytes_changed_functors.emplace_back([object, functor](uint64_t value) -> void{
                std::invoke(functor, object, value);
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object> object
         * \param functor void (Object::*functor)(std::shared_ptr<Response>)
         */
        template<class Object> void notifyWhenFinished(std::weak_ptr<Object> object, void (Object::*functor)(std::shared_ptr<Response>)){
            m_notify_when_finished_functors.emplace_back([object, functor](std::shared_ptr<Response> request) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, request);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)(std::shared_ptr<Response>)
         */
        template<class Object> void notifyWhenFinished(Object* object, void (Object::*functor)(std::shared_ptr<Response>)){
            m_notify_when_finished_functors.emplace_back([object, functor](std::shared_ptr<Response> request) -> void{
                std::invoke(functor, object, request);
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object> object
         * \param functor void (Object::*functor)()
         */
        template<class Object> void notifyWhenFinished(std::weak_ptr<Object> object, void (Object::*functor)()){
            m_notify_when_finished_void_functors.emplace_back([object, functor]() -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template<class Object> void notifyWhenFinished(Object* object, void (Object::*functor)()){
            m_notify_when_finished_void_functors.emplace_back([object, functor]() -> void{ std::invoke(functor, object); });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param functor const std::function<void(std::shared_ptr<Response>)>&
         */
        void notifyWhenFinished(const std::function<void(std::shared_ptr<Response>)>& functor){
            m_notify_when_finished_functors.emplace_back(functor);
        }

        /**
         * \brief Registers callback functions which will be invoked when network request status had been changed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object> object
         * \param functor void (Object::*functor)(std::pair<std::string, tristan::network::Status>) where std::string if for requests' UUID
         */
        template<class Object> void notifyWhenStatusChanged(std::weak_ptr<Object> object, void (Object::*functor)(std::pair<std::string, tristan::network::Status>)){
            m_notify_when_status_changed_functors.emplace_back([object, functor](std::pair<std::string, tristan::network::Status> status) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, status);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had been changed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)(std::pair<std::string, tristan::network::Status>) where std::string if for requests' UUID
         */
        template<class Object> void notifyWhenStatusChanged(Object* object, void (Object::*functor)(std::pair<std::string, tristan::network::Status>)){
            m_notify_when_status_changed_functors.emplace_back([object, functor](std::pair<std::string, tristan::network::Status> status) -> void{
                std::invoke(functor, object, status);
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request is paused.
         * \param functor std::function<void()> functor
         */
        void notifyWhenPaused(std::function<void()> functor){
            m_notify_when_paused_void_functors.emplace_back(functor);
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request is paused.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template<class Object> void notifyWhenPaused(std::weak_ptr<Object> object, void (Object::*functor)()){
            m_notify_when_paused_void_functors.emplace_back([object, functor]() -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request is paused.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template<class Object> void notifyWhenPaused(Object* object, void (Object::*functor)()){
            m_notify_when_paused_void_functors.emplace_back([object, functor]() -> void{ std::invoke(functor, object); });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request is paused.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object> object
         * \param functor void (Object::*functor)(std::shared_ptr<Response>)
         */
        template<class Object> void notifyWhenPaused(std::weak_ptr<Object> object, void (Object::*functor)(std::shared_ptr<NetworkRequest>)){
            m_notify_when_paused_functors.emplace_back([object, functor](std::shared_ptr<NetworkRequest> request) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, request);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request is paused.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)(std::shared_ptr<Response>)
         */
        template<class Object> void notifyWhenPaused(Object* object, void (Object::*functor)(std::shared_ptr<NetworkRequest>)){
            m_notify_when_paused_functors.emplace_back([object, functor](std::shared_ptr<NetworkRequest> request) -> void{
                std::invoke(functor, object, request);
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request is resumed.
         * \param functor std::function<void()> functor
         */
        void notifyWhenResumed(std::function<void()> functor){
            m_notify_when_resumed_void_functors.emplace_back(functor);
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request is resumed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template<class Object> void notifyWhenResumed(std::weak_ptr<Object> object, void (Object::*functor)()){
            m_notify_when_resumed_void_functors.emplace_back([object, functor]() -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request is resumed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template<class Object> void notifyWhenResumed(Object* object, void (Object::*functor)()){
            m_notify_when_resumed_void_functors.emplace_back([object, functor]() -> void{ std::invoke(functor, object); });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request is resumed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(std::shared_ptr<Response>
         */
        template<class Object> void notifyWhenResumed(std::weak_ptr<Object> object, void (Object::*functor)(std::shared_ptr<NetworkRequest>)
        ){
            m_notify_when_resumed_functors.emplace_back([object, functor](std::shared_ptr<NetworkRequest> request) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, request);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request is resumed.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)(std::shared_ptr<Response>
         */
        template<class Object> void notifyWhenResumed(Object* object, void (Object::*functor)(std::shared_ptr<NetworkRequest>)){
            m_notify_when_resumed_functors.emplace_back([object, functor](std::shared_ptr<NetworkRequest> request) -> void{
                std::invoke(functor, object, request);
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when error occurred.
         * \note Registered callbacks will be invoked when there are any errors related to network connection e.g. resolver wasn't able to resolve the host. That is, e.g. http errors, are not considered here as an error.
         * \param functor std::function<void(std::pair<std::string, std::error_code>)> where std::string stores UUID of the request.
         */
        void notifyWhenError(std::function<void(std::pair<std::string, std::error_code>)> functor){
            m_notify_when_error_functors.emplace_back(functor);
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when error occurred.
         * \note Registered callbacks will be invoked when there are any errors related to network connection e.g. resolver wasn't able to resolve the host. That is, e.g. http errors, are not considered here as an error.
         * \tparam Object Type which holds the function member to invoke.
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::pair<std::string, std::error_code>&) where std::string stores UUID of the request.
         */
        template<class Object> void notifyWhenError(std::weak_ptr<Object> object, void (Object::*functor)(const std::pair<std::string, std::error_code>&)){
            m_notify_when_error_functors.emplace_back([object, functor](const std::pair<std::string, std::error_code>& error) -> void{
                if (auto l_object = object.lock()){
                    std::invoke(functor, l_object, error);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when error occurred.
         * \note Registered callbacks will be invoked when there are any errors related to network connection e.g. resolver wasn't able to resolve the host. That is, e.g. http errors, are not considered here as an error.
         * \tparam Object Type which holds the function member to invoke.
         * \param object Object*
         * \param functor void (Object::*functor)(const std::pair<std::string, std::error_code>&) where std::string stores UUID of the request.
         */
        template<class Object> void notifyWhenError(Object* object, void (Object::*functor)(const std::pair<std::string, std::error_code>&)){
            m_notify_when_error_functors.emplace_back([object, functor](const std::pair<std::string, std::error_code>& error) -> void{
                std::invoke(functor, object, error);
            });
        }

        /**
        * \brief Sets priority of the request
        * \param priority Priority
        */
        void setPriority(Priority priority){ m_priority = priority; }

        /**
         * \brief Cancels the request execution.
         */
        void cancel(){ m_cancelled.store(true); }

        /**
         * \brief Pauses request processing
         */
        void pauseProcessing(){ m_paused.store(true); }

        /**
         * \brief Resumes request processing
         */
        void continueProcessing(){ m_paused.store(false); }

        /**
         * \brief Base function for request processing. Should be overloaded in each derived class.
         */
        virtual void doRequest(){}

        /**
         * \brief UUID getter.
         * \return const std::string&
         */
        [[nodiscard]] auto uuid() const noexcept -> const std::string&{ return m_uuid; }

        /**
         * \brief Error getter.
         * \return const std::string&
         */
        [[nodiscard]] auto error() const noexcept -> const std::error_code&{ return m_error; }

        /**
         * \brief Status getter.
         * \return const std::string&
         */
        [[nodiscard]] auto status() const noexcept -> Status{ return m_status; }

        /**
         * \brief Paused state getter.
         * \return const std::string&
         */
        [[nodiscard]] auto paused() const noexcept -> bool{ return m_paused.load(); }

        /**
         * \brief Returns response got while request was processed.
         * \note If request was not processed - doRequest function will be invoked.
         * \return
         */
        [[nodiscard]] auto response() -> std::shared_ptr<Response>{
            if (m_status != tristan::network::Status::DONE){
                this->doRequest();
            }
            return m_response;
        }

        /**
         * \brief Priority getter.
         * \return const std::string&
         */
        [[nodiscard]] auto priority() -> Priority{ return m_priority; }

        /**
         * \brief Prepares data which should be sent to the remote. Should be overloaded in each derived class.
         * \return std::string
         */
        [[nodiscard]] virtual auto prepareRequest() const -> std::string{ return { }; }

      protected:
        /**
         * \brief Constructor
         * \param uri Uri
         */
        explicit NetworkRequest(Uri uri) :
                m_uri(std::move(uri)),
                m_uuid(utility::getUuid()),
                m_bytes_to_read(0),
                m_bytes_read(0),
                m_status(Status::WAITING),
                m_priority(Priority::NORMAL),
                m_output_to_file(false),
                m_paused(false),
                m_cancelled(false){}

        /**
         * \overload
         * \brief Constructor
         * \param ip const std::string&
         * \param port uint16_t
         */
        NetworkRequest(const std::string& ip, uint16_t port) :
                m_uuid(utility::getUuid()),
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
        std::vector<std::function<void(std::pair<std::string, tristan::network::Status>)>> m_notify_when_status_changed_functors;
        std::vector<std::function<void(std::shared_ptr<Response>)>> m_notify_when_paused_functors;
        std::vector<std::function<void(void)>> m_notify_when_paused_void_functors;
        std::vector<std::function<void(std::shared_ptr<Response>)>> m_notify_when_resumed_functors;
        std::vector<std::function<void(void)>> m_notify_when_resumed_void_functors;
        std::vector<std::function<void(void)>> m_notify_when_finished_void_functors;
        std::vector<std::function<void(const std::pair<std::string, std::error_code>&)>> m_notify_when_error_functors;

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

        void _notifyWhenStatusChanged(){
            if (m_notify_when_status_changed_functors.empty()){
                return;
            }
            for (const auto& functor: m_notify_when_status_changed_functors){
                functor({ m_uuid,
                          m_status
                        });
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
