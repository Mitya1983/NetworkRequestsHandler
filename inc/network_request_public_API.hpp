#ifndef NETWORK_REQUEST_PUBLIC_API_HPP
#define NETWORK_REQUEST_PUBLIC_API_HPP

#include "network_request_protected_API.hpp"

namespace tristan::network {
    class NetworkRequestPublicAPI : virtual public NetworkRequestProtectedAPI {
    public:
        friend bool operator<(const tristan::network::NetworkRequestPublicAPI& left, const tristan::network::NetworkRequestPublicAPI& right) {
            return left.m_priority < right.m_priority;
        }

        friend bool operator>(const tristan::network::NetworkRequestPublicAPI& left, const tristan::network::NetworkRequestPublicAPI& right) {
            return left.m_priority > right.m_priority;
        }

        /**
         * \brief Constructor
         * \param url Uri&&
         */
        explicit NetworkRequestPublicAPI(Url&& url);

        /**
         * \overload
         * \brief Constructor
         * \param uri const Url&
         */
        explicit NetworkRequestPublicAPI(const Url& url);

        NetworkRequestPublicAPI() = delete;

        NetworkRequestPublicAPI(const NetworkRequestPublicAPI& other) = delete;

        NetworkRequestPublicAPI(NetworkRequestPublicAPI&& other) noexcept = delete;

        NetworkRequestPublicAPI& operator=(const NetworkRequestPublicAPI& other) = delete;

        NetworkRequestPublicAPI& operator=(NetworkRequestPublicAPI&& other) noexcept = delete;

        ~NetworkRequestPublicAPI() override = default;
        /**
         * \brief Sets priority of the request
         * \param priority Priority
         */
        void setPriority(Priority priority);

        /**
         * \brief Sets total amount of bytes which should be read from socket
         * @param bytes uint64_t
         */
        void setBytesToRead(uint64_t bytes);

        /**
         * \brief Sets if request should be processed as Ssl. By default is set to false.
         * @param value bool
         */
        void setSSL(bool value = true);

        /**
         * Sets delimiter which will be used to truncate incoming stream.
         * @param delimiter std::vector< uint8_t >&&
         */
        void setResponseDelimiter(std::vector< uint8_t >&& delimiter);

        /**
         * Sets delimiter which will be used to truncate incoming stream.
         * @param delimiter const std::vector< uint8_t >&
         */
        void setResponseDelimiter(const std::vector< uint8_t >& delimiter);

        /**
         * \brief Sets file where response fom remote will be stored
         * \param path std::filesystem::path&&
         */
        void outputToFile(std::filesystem::path&& path);

        /**
         * \overload
         * \brief Sets file where response fom remote will be stored
         * \param path const std::filesystem::path&
         */
        void outputToFile(const std::filesystem::path& path);

        /**
         * \brief Cancels the request execution.
         */
        void cancel();

        /**
         * \brief Pauses request processing
         */
        void pauseProcessing();

        /**
         * \brief Resumes request processing
         */
        void continueProcessing();

        /**
         * \brief Set request data using move.
         * \param request_data std::vector<uint8_t>&&
         */
        void setRequest(std::vector< uint8_t >&& request_data);

        /**
         * \brief Set request data using copy.
         * \param request_data const std::vector<uint8_t>&
         */
        void setRequest(const std::vector< uint8_t >& request_data);

        /**
         * \brief Returned UUID of a request.
         * \return const std::string&
         */
        [[nodiscard]] auto uuid() const noexcept -> const std::string&;

        /**
         * \brief Returns url which was/will be used for request precessing
         * \return const Url&
         */
        [[nodiscard]] auto url() const noexcept -> const Url&;

        /**
         * \brief Returns error which is held by the request.
         * \return const std::error_code&
         */
        [[nodiscard]] auto error() const noexcept -> const std::error_code&;

        /**
         * \brief Returns status of the request
         * \return const Status
         */
        [[nodiscard]] auto status() const noexcept -> Status;

        /**
         * \brief Returns whether request is paused.
         * \return bool
         */
        [[nodiscard]] auto isPaused() const noexcept -> bool;

        /**
         * \brief Returns whether request is canceled.
         * \return bool
         */
        [[nodiscard]] auto isCanceled() const noexcept -> bool;

        /**
         * \brief Priority getter.
         * \return const std::string&
         */
        [[nodiscard]] auto priority() const noexcept -> Priority;

        /**
         * \brief Returns total amount of bytes which should be read from socket
         * @return uint64_t
         */
        [[nodiscard]] auto bytesToRead() const noexcept -> uint64_t;

        /**
         * \brief Indicates if request chould be processed using ssl.
         * @return bool
         */
        [[nodiscard]] auto isSSL() const noexcept -> bool;

        /**
         * Returns delimiter which will be used to truncate the incoming stream
         * @return char
         */
        [[nodiscard]] auto responseDelimiter() const noexcept -> const std::vector< uint8_t >&;

        /**
         * \brief Prepares data which should be sent to the remote.
         * \note The protected member m_request data is provided to prepare request.
         * \return const std::vector<uint8_t>&
         */
        [[nodiscard]] virtual auto requestData() -> const std::vector< uint8_t >& = 0;

        /**
         * \brief Returns response which was returned by the remote.
         * If request was not processed an empty std::shared_ptr is returned.
         * \return std::shared_ptr<std::vector<uint8_t>>
         */
        [[nodiscard]] auto response() -> std::shared_ptr< NetworkResponse >;

        /**
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \param functor std::function<void(uint64_t)>&&
         */
        void addReadBytesValueChangedCallback(std::function< void(uint64_t) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(uint64_t)
         */
        template < class Object > void addReadBytesValueChangedCallback(std::weak_ptr< Object > object, void (Object::*functor)(uint64_t)) {
            m_read_bytes_changed_callback_functors.emplace_back([object, functor](uint64_t value) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, value);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(uint64_t)
         */
        template < class Object > void addReadBytesValueChangedCallback(Object* object, void (Object::*functor)(uint64_t)) {
            m_read_bytes_changed_callback_functors.emplace_back([object, functor](uint64_t value) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, value);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \param functor std::function<void(uint64_t)>&&
         */
        void addReadBytesValueChangedCallback(std::function< void(const std::string&, uint64_t) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&, uint64_t)
         */
        template < class Object > void addReadBytesValueChangedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&, uint64_t)) {
            m_read_bytes_changed_callback_functors.emplace_back([object, functor](const std::string& id, uint64_t value) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id, value);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&, uint64_t)
         */
        template < class Object > void addReadBytesValueChangedCallback(Object* object, void (Object::*functor)(const std::string&, uint64_t)) {
            m_read_bytes_changed_callback_functors.emplace_back([object, functor](const std::string& id, uint64_t value) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id, value);
                }
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param functor std::function<void()>&& functor
         */
        void addFinishedCallback(std::function< void() >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addFinishedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
            m_finished_void_callback_functors.emplace_back([object, functor]() -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addFinishedCallback(Object* object, void (Object::*functor)()) {
            m_finished_void_callback_functors.emplace_back([object, functor]() -> void {
                if (object != nullptr) {
                    std::invoke(functor, object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param functor std::function<void(const std::string&)>&& functor
         */
        void addFinishedCallback(std::function< void(const std::string&) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addFinishedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
            m_finished_with_id_callback_functors.emplace_back([object, functor](const std::string& id) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addFinishedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_finished_with_id_callback_functors.emplace_back([object, functor](const std::string& id) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param functor std::function<void(std::shared_ptr<std::vector<uint8_t>>)>&& functor
         */
        void addFinishedCallback(std::function< void(std::shared_ptr< NetworkResponse >) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object > void addFinishedCallback(std::weak_ptr< Object > object, void (Object::*functor)(std::shared_ptr< NetworkResponse >)) {
            m_finished_with_response_callback_functors.emplace_back([object, functor](std::shared_ptr< std::vector< uint8_t > > response) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, response);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object > void addFinishedCallback(Object* object, void (Object::*functor)(std::shared_ptr< NetworkResponse >)) {
            m_finished_with_id_callback_functors.emplace_back([object, functor](std::shared_ptr< std::vector< uint8_t > > response) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, response);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param functor std::function<void(const std::string&, std::shared_ptr<std::vector<uint8_t>>)>&& functor
         */
        void addFinishedCallback(std::function< void(const std::string&, std::shared_ptr< NetworkResponse >) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&, std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object >
        void addFinishedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&, std::shared_ptr< NetworkResponse >)) {
            m_finished_with_response_callback_functors.emplace_back(
                [object, functor](const std::string& id, std::shared_ptr< std::vector< uint8_t > > response) -> void {
                    if (auto l_object = object.lock()) {
                        std::invoke(functor, l_object, id, response);
                    }
                });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&, std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object > void addFinishedCallback(Object* object, void (Object::*functor)(const std::string&, std::shared_ptr< NetworkResponse >)) {
            m_finished_with_id_callback_functors.emplace_back(
                [object, functor](const std::string& id, std::shared_ptr< std::vector< uint8_t > > response) -> void {
                    if (object != nullptr) {
                        std::invoke(functor, object, id, response);
                    }
                });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \param functor std::function<void()>&& functor
         */
        void addStatusChangedCallback(std::function< void() >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addStatusChangedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
            m_finished_void_callback_functors.emplace_back([object, functor]() -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addStatusChangedCallback(Object* object, void (Object::*functor)()) {
            m_finished_void_callback_functors.emplace_back([object, functor]() -> void {
                if (object != nullptr) {
                    std::invoke(functor, object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \param functor std::function<void(const std::string&)>&& functor
         */
        void addStatusChangedCallback(std::function< void(const std::string&) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addStatusChangedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
            m_finished_with_id_callback_functors.emplace_back([object, functor](const std::string& id) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addStatusChangedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_finished_with_id_callback_functors.emplace_back([object, functor](const std::string& id) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \param functor std::function<void(Status)>&& functor
         */
        void addStatusChangedCallback(std::function< void(Status) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(Status)
         */
        template < class Object > void addStatusChangedCallback(std::weak_ptr< Object > object, void (Object::*functor)(Status)) {
            m_finished_with_response_callback_functors.emplace_back([object, functor](Status status) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, status);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(Status)
         */
        template < class Object > void addStatusChangedCallback(Object* object, void (Object::*functor)(Status)) {
            m_finished_with_id_callback_functors.emplace_back([object, functor](Status status) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, status);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \param functor std::function<void(const std::string&, Status)>&& functor
         */
        void addStatusChangedCallback(std::function< void(const std::string&, Status) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&, Status)
         */
        template < class Object > void addStatusChangedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&, Status)) {
            m_finished_with_response_callback_functors.emplace_back([object, functor](const std::string& id, Status status) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id, status);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&, Status)
         */
        template < class Object > void addStatusChangedCallback(Object* object, void (Object::*functor)(const std::string&, Status)) {
            m_finished_with_id_callback_functors.emplace_back([object, functor](const std::string& id, Status status) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id, status);
                }
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \param functor std::function<void()>&&
         */
        void addPausedCallback(std::function< void() >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addPausedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
            m_paused_void_functors.emplace_back([object, functor]() -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addPausedCallback(Object* object, void (Object::*functor)()) {
            m_paused_void_functors.emplace_back([object, functor]() -> void {
                if (object != nullptr) {
                    std::invoke(functor, object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \param functor std::function<void(const std::string&)>&&
         */
        void addPausedCallback(std::function< void(const std::string&) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addPausedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
            m_paused_void_functors.template emplace_back([object, functor](const std::string& id) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addPausedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_paused_void_functors.template emplace_back([object, functor](const std::string& id) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id);
                }
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \param functor std::function<void()>&&
         */
        void addResumedCallback(std::function< void() >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addResumedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
            m_resumed_void_functors.emplace_back([object, functor]() -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addResumedCallback(Object* object, void (Object::*functor)()) {
            m_resumed_void_functors.emplace_back([object, functor]() -> void {
                if (object != nullptr) {
                    std::invoke(functor, object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \param functor std::function<void(const std::string&)>&&
         */
        void addResumedCallback(std::function< void(const std::string&) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addResumedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
            m_resumed_with_id_functors.template emplace_back([object, functor](const std::string& id) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addResumedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_resumed_with_id_functors.template emplace_back([object, functor](const std::string& id) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id);
                }
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \param functor std::function<void()>&&
         */
        void addCanceledCallback(std::function< void() >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addCanceledCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
            m_canceled_void_functors.emplace_back([object, functor]() -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addCanceledCallback(Object* object, void (Object::*functor)()) {
            m_canceled_void_functors.emplace_back([object, functor]() -> void {
                if (object != nullptr) {
                    std::invoke(functor, object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \param functor std::function<void(const std::string&)>&&
         */
        void addCanceledCallback(std::function< void(const std::string&) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addCanceledCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
            m_canceled_with_id_functors.template emplace_back([object, functor](const std::string& id) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addCanceledCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_canceled_with_id_functors.template emplace_back([object, functor](const std::string& id) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id);
                }
            });
        }

        /**
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \param functor std::function<void()>&&
         */
        void addFailedCallback(std::function< void() >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addFailedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
            m_failed_void_callback_functors.emplace_back([object, functor]() -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)()
         */
        template < class Object > void addFailedCallback(Object* object, void (Object::*functor)()) {
            m_failed_void_callback_functors.emplace_back([object, functor]() -> void {
                if (object != nullptr) {
                    std::invoke(functor, object);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \param functor std::function<void(const std::string&)>&&
         */
        void addFailedCallback(std::function< void(const std::string&) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addFailedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
            m_failed_with_id_callback_functors.template emplace_back([object, functor](const std::string& id) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&)
         */
        template < class Object > void addFailedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_failed_with_id_callback_functors.template emplace_back([object, functor](const std::string& id) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \param functor std::function<void(std::error_code)>&&
         */
        void addFailedCallback(std::function< void(std::error_code) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(std::error_code)
         */
        template < class Object > void addFailedCallback(std::weak_ptr< Object > object, void (Object::*functor)(std::error_code)) {
            m_failed_with_error_code_callback_functors.template emplace_back([object, functor](std::error_code error_code) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, error_code);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(std::error_code)
         */
        template < class Object > void addFailedCallback(Object* object, void (Object::*functor)(std::error_code)) {
            m_failed_with_error_code_callback_functors.emplace_back([object, functor](std::error_code error_code) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, error_code);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \param functor std::function<void(const std::string&, std::error_code)>&&
         */
        void addFailedCallback(std::function< void(const std::string&, std::error_code) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&, std::error_code)
         */
        template < class Object > void addFailedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&, std::error_code)) {
            m_failed_with_id_and_error_code_callback_functors.emplace_back([object, functor](const std::string& id, std::error_code error_code) -> void {
                if (auto l_object = object.lock()) {
                    std::invoke(functor, l_object, id, error_code);
                }
            });
        }

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param object Object*
         * \param functor void (Object::*functor)(const std::string&, std::error_code)
         */
        template < class Object > void addFailedCallback(Object* object, void (Object::*functor)(const std::string&, std::error_code)) {
            m_failed_with_id_and_error_code_callback_functors.emplace_back([object, functor](const std::string& id, std::error_code error_code) -> void {
                if (object != nullptr) {
                    std::invoke(functor, object, id, error_code);
                }
            });
        }
    };
}  // namespace tristan::network

#endif  //NETWORK_REQUEST_PUBLIC_API_HPP
