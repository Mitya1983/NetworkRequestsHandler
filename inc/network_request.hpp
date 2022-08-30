#ifndef NETWORK_REQUEST_HPP
#define NETWORK_REQUEST_HPP

#include "url.hpp"
#include "network_utility.hpp"
#include "network_response.hpp"
#include "network_error.hpp"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <concepts>

namespace tristan::network {
    /**
     * \enum Status
     * \brief List of possible request statuses
     */
    enum class Status : uint8_t {
        /**
         * \brief Request is created but not being processed yet
         */
        WAITING,

        /**
         * \brief Request is being processed by request handler
         */
        PROCESSED,

        /**
         * \brief Network request handler failed to handle the request
         */
        ERROR,

        /**
         * \brief Request waits for data been downloaded
         */
        PENDING_DOWNLOAD,

        /**
         * \brief Request downloads data
         */
        DOWNLOADING,

        /**
         * \brief Request processing was paused
         */
        PAUSED,

        /**
         * \brief Request processing was resumed
         */
        RESUMED,

        /**
         * \brief Canceled signal was received
         */
        CANCELED,

        /**
         * \brief Request had been processed
         */
        DONE
    };
    /**
     * \enum Priority
     * \brief List of possible priority values
     */
    enum class Priority : uint8_t {
        /**
         * \brief Low priority
         */
        LOW,    /**
                 * \brief Normal priority
                 */
        NORMAL, /**
                 * \brief High priority
                 */
        HIGH,   /**
                 * \brief Request will be processed out of queue with out any delay
                 */
        OUT_OF_QUEUE
    };

    /**
     * \class NetworkRequest
     * \brief Used as a base class for network request classes. E.g. HttpRequest.
     */
    class NetworkRequest {

        //        friend class NetworkRequestsHandler;

    public:
        friend bool operator<(const NetworkRequest& left, const NetworkRequest& right) {
            return left.m_priority < right.m_priority;
        }

        friend bool operator>(const NetworkRequest& left, const NetworkRequest& right) {
            return left.m_priority > right.m_priority;
        }

        /**
         * \brief Constructor
         * \param url Uri&&
         */
        explicit NetworkRequest(Url&& url);

        /**
         * \overload
         * \brief Constructor
         * \param uri const Url&
         */
        explicit NetworkRequest(const Url& url);

        NetworkRequest() = delete;

        NetworkRequest(const NetworkRequest& other) = delete;

        NetworkRequest(NetworkRequest&& other) noexcept = delete;

        NetworkRequest& operator=(const NetworkRequest& other) = delete;

        NetworkRequest& operator=(NetworkRequest&& other) noexcept = delete;

        virtual ~NetworkRequest() = default;

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
         * \brief Sets if request should be processed as SSL. By default is set to false.
         * @param value bool
         */
        void setSSL(bool value = true);

        /**
         * Sets delimiter which will be used to truncate incoming stream.
         * @param delimiter
         */
        void setResponseDelimiter(std::string delimiter);

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
        [[nodiscard]] auto responseDelimiter() const noexcept ->const std::string&;

        /**
         * \brief Prepares data which should be sent to the remote.
         * \note The protected member m_request data is provided to prepare request.
         * \return const std::vector<uint8_t>&
         */
        [[nodiscard]] virtual auto requestData() -> const std::vector< uint8_t >&;

        /**
         * \brief Returns response which was returned by the remote.
         * If request was not processed an empty std::shared_ptr is returned.
         * \return std::shared_ptr<std::vector<uint8_t>>
         */
        [[nodiscard]] auto responseData() -> std::shared_ptr< std::vector< uint8_t > >;

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
        template < class Object >
        void addReadBytesValueChangedCallback(std::weak_ptr< Object > object,
                                              void (Object::*functor)(uint64_t)) {
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
        template < class Object >
        void addReadBytesValueChangedCallback(Object* object, void (Object::*functor)(uint64_t)) {
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
        template < class Object >
        void addReadBytesValueChangedCallback(std::weak_ptr< Object > object,
                                              void (Object::*functor)(const std::string&, uint64_t)) {
            m_read_bytes_changed_callback_functors.emplace_back(
                [object, functor](const std::string& id, uint64_t value) -> void {
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
        template < class Object >
        void addReadBytesValueChangedCallback(Object* object,
                                              void (Object::*functor)(const std::string&, uint64_t)) {
            m_read_bytes_changed_callback_functors.emplace_back(
                [object, functor](const std::string& id, uint64_t value) -> void {
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
        template < class Object >
        void addFinishedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
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
        template < class Object >
        void addFinishedCallback(std::weak_ptr< Object > object,
                                 void (Object::*functor)(const std::string&)) {
            m_finished_with_id_callback_functors.emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addFinishedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_finished_with_id_callback_functors.emplace_back(
                [object, functor](const std::string& id) -> void {
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
        void addFinishedCallback(std::function< void(std::shared_ptr< std::vector< uint8_t > >) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object >
        void addFinishedCallback(std::weak_ptr< Object > object,
                                 void (Object::*functor)(std::shared_ptr< std::vector< uint8_t > >)) {
            m_finished_with_response_callback_functors.emplace_back(
                [object, functor](std::shared_ptr< std::vector< uint8_t > > response) -> void {
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
        template < class Object >
        void addFinishedCallback(Object* object,
                                 void (Object::*functor)(std::shared_ptr< std::vector< uint8_t > >)) {
            m_finished_with_id_callback_functors.emplace_back(
                [object, functor](std::shared_ptr< std::vector< uint8_t > > response) -> void {
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
        void addFinishedCallback(
            std::function< void(const std::string&, std::shared_ptr< std::vector< uint8_t > >) >&& functor);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param object std::weak_ptr<Object>
         * \param functor void (Object::*functor)(const std::string&, std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object >
        void addFinishedCallback(std::weak_ptr< Object > object,
                                 void (Object::*functor)(const std::string&,
                                                         std::shared_ptr< std::vector< uint8_t > >)) {
            m_finished_with_response_callback_functors.emplace_back(
                [object, functor](const std::string& id,
                                  std::shared_ptr< std::vector< uint8_t > > response) -> void {
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
        template < class Object >
        void addFinishedCallback(Object* object,
                                 void (Object::*functor)(const std::string&,
                                                         std::shared_ptr< std::vector< uint8_t > >)) {
            m_finished_with_id_callback_functors.emplace_back(
                [object, functor](const std::string& id,
                                  std::shared_ptr< std::vector< uint8_t > > response) -> void {
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
        template < class Object >
        void addStatusChangedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
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
        template < class Object >
        void addStatusChangedCallback(std::weak_ptr< Object > object,
                                      void (Object::*functor)(const std::string&)) {
            m_finished_with_id_callback_functors.emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addStatusChangedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_finished_with_id_callback_functors.emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addStatusChangedCallback(std::weak_ptr< Object > object, void (Object::*functor)(Status)) {
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
        template < class Object >
        void addStatusChangedCallback(Object* object, void (Object::*functor)(Status)) {
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
        template < class Object >
        void addStatusChangedCallback(std::weak_ptr< Object > object,
                                      void (Object::*functor)(const std::string&, Status)) {
            m_finished_with_response_callback_functors.emplace_back(
                [object, functor](const std::string& id, Status status) -> void {
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
        template < class Object >
        void addStatusChangedCallback(Object* object, void (Object::*functor)(const std::string&, Status)) {
            m_finished_with_id_callback_functors.emplace_back(
                [object, functor](const std::string& id, Status status) -> void {
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
        template < class Object >
        void addPausedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
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
        template < class Object >
        void addPausedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
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
        template < class Object >
        void addPausedCallback(Object* object, void (Object::*functor)(const std::string&)) {
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
        template < class Object >
        void addResumedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
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
        template < class Object >
        void addResumedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
            m_resumed_with_id_functors.template emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addResumedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_resumed_with_id_functors.template emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addCanceledCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
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
        template < class Object >
        void addCanceledCallback(std::weak_ptr< Object > object,
                                 void (Object::*functor)(const std::string&)) {
            m_canceled_with_id_functors.template emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addCanceledCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_canceled_with_id_functors.template emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addFailedCallback(std::weak_ptr< Object > object, void (Object::*functor)()) {
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
        template < class Object >
        void addFailedCallback(std::weak_ptr< Object > object, void (Object::*functor)(const std::string&)) {
            m_failed_with_id_callback_functors.template emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addFailedCallback(Object* object, void (Object::*functor)(const std::string&)) {
            m_failed_with_id_callback_functors.template emplace_back(
                [object, functor](const std::string& id) -> void {
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
        template < class Object >
        void addFailedCallback(std::weak_ptr< Object > object, void (Object::*functor)(std::error_code)) {
            m_failed_with_error_code_callback_functors.template emplace_back(
                [object, functor](std::error_code error_code) -> void {
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
        template < class Object >
        void addFailedCallback(Object* object, void (Object::*functor)(std::error_code)) {
            m_failed_with_error_code_callback_functors.emplace_back(
                [object, functor](std::error_code error_code) -> void {
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
        template < class Object >
        void addFailedCallback(std::weak_ptr< Object > object,
                               void (Object::*functor)(const std::string&, std::error_code)) {
            m_failed_with_id_and_error_code_callback_functors.emplace_back(
                [object, functor](const std::string& id, std::error_code error_code) -> void {
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
        template < class Object >
        void addFailedCallback(Object* object, void (Object::*functor)(const std::string&, std::error_code)) {
            m_failed_with_id_and_error_code_callback_functors.emplace_back(
                [object, functor](const std::string& id, std::error_code error_code) -> void {
                    if (object != nullptr) {
                        std::invoke(functor, object, id, error_code);
                    }
                });
        }

        class ProtectedMembers {
            friend class NetworkRequestsHandler;
            friend class Downloader;

        public:
            /**
             * \brief Adds data to response data
             * \param network_request NetworkRequest&
             * \param data std::vector<uint8_t>&&
             */
            static void pAddResponseData(NetworkRequest& network_request, std::vector< uint8_t >&& data);

            /**
             * \overload
             * \brief Adds data to response data
             * \param network_request const std::shared_ptr<NetworkRequest>&
             * \param data std::vector<uint8_t>&&
             */
            static void pAddResponseData(std::shared_ptr< NetworkRequest > network_request,
                                         std::vector< uint8_t >&& data);

            /**
             * \brief Sets current request status
             * \param network_request NetworkRequest&
             * \param status Status
             */
            static void pSetStatus(NetworkRequest& network_request, Status status);

            /**
             * \overload
             * \brief Sets current request status
             * \param network_request const std::shared_ptr<NetworkRequest>&
             * \param status Status
             */
            static void pSetStatus(std::shared_ptr< NetworkRequest > network_request, Status status);

            /**
             * \brief Sets error
             * \param network_request NetworkRequest&
             * \param error_code std::error_code
             */
            static void pSetError(NetworkRequest& network_request, std::error_code error_code);

            /**
             * \overload
             * \brief Sets error
             * \param network_request const std::shared_ptr<NetworkRequest>&
             * \param error_code std::error_code
             */
            static void pSetError(std::shared_ptr< NetworkRequest > network_request,
                                  std::error_code error_code);
        };

    protected:
        std::vector< uint8_t > m_request_data;
        Url m_url;

        /**
         * \brief Stores protected modifying API for RequestHandler class and derived classes
         */

    private:
        std::ofstream m_output_file;

        std::filesystem::path m_output_path;

        std::string m_uuid;
        std::string m_delimiter;

        std::vector< std::function< void(uint64_t) > > m_read_bytes_changed_callback_functors;
        std::vector< std::function< void(const std::string&, uint64_t) > >
            m_read_bytes_changed_with_id_callback_functors;
        std::vector< std::function< void() > > m_finished_void_callback_functors;
        std::vector< std::function< void(const std::string&) > > m_finished_with_id_callback_functors;
        std::vector< std::function< void(std::shared_ptr< std::vector< uint8_t > >) > >
            m_finished_with_response_callback_functors;
        std::vector< std::function< void(const std::string&, std::shared_ptr< std::vector< uint8_t > >) > >
            m_finished_with_id_and_response_callback_functors;
        std::vector< std::function< void() > > m_status_changed_void_callback_functors;
        std::vector< std::function< void(const std::string&) > > m_status_changed_with_id_callback_functors;
        std::vector< std::function< void(Status) > > m_status_changed_with_status_callback_functors;
        std::vector< std::function< void(const std::string&, Status) > >
            m_status_changed_with_id_and_status_callback_functors;
        std::vector< std::function< void() > > m_paused_void_functors;
        std::vector< std::function< void(const std::string&) > > m_paused_with_id_functors;
        std::vector< std::function< void() > > m_resumed_void_functors;
        std::vector< std::function< void(const std::string&) > > m_resumed_with_id_functors;
        std::vector< std::function< void() > > m_canceled_void_functors;
        std::vector< std::function< void(const std::string&) > > m_canceled_with_id_functors;
        std::vector< std::function< void() > > m_failed_void_callback_functors;
        std::vector< std::function< void(const std::string&) > > m_failed_with_id_callback_functors;
        std::vector< std::function< void(std::error_code) > > m_failed_with_error_code_callback_functors;
        std::vector< std::function< void(const std::string&, std::error_code) > >
            m_failed_with_id_and_error_code_callback_functors;

        std::error_code m_error;
        std::shared_ptr< std::vector< uint8_t > > m_response_data;

        uint64_t m_bytes_to_read;
        uint64_t m_bytes_read;

        Status m_status;
        Priority m_priority;
        std::atomic< bool > m_paused;
        std::atomic< bool > m_canceled;
        bool m_output_to_file;
        bool m_ssl;

        void pNotifyWhenBytesReadChanged();

        void pNotifyWhenStatusChanged();

        void pNotifyWhenPaused();

        void pNotifyWhenResumed();

        void pNotifyWhenCanceled();

        void pNotifyWhenFinished();

        void pNotifyWhenFailed();
    };

}  // namespace tristan::network

#endif  // NETWORK_REQUEST_HPP
