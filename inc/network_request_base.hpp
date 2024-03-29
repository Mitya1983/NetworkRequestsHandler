#ifndef NETWORK_REQUEST_BASE_HPP
#define NETWORK_REQUEST_BASE_HPP

#include "url.hpp"
#include "network_utility.hpp"

#include <filesystem>
#include <vector>
#include <functional>
#include <atomic>
#include <fstream>
#include <chrono>

namespace tristan::network {

    class NetworkResponse;
    class FriendClassesFunctions;

    namespace private_ {
        class NetworkRequestHandlerImpl;
        class SyncNetworkRequestHandlerImpl;
        class AsyncNetworkRequestHandlerImpl;
        class AsyncRequestHandler;
    } //End of private_ namespace

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
        WRITING,

        /**
         * \brief Request downloads data
         */
        READING,

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

    class NetworkRequestBase {

    private:
        class FriendClassesAPI {
            friend class NetworkRequestBase;
            friend class private_::NetworkRequestHandlerImpl;
            friend class private_::SyncNetworkRequestHandlerImpl;
            friend class private_::AsyncNetworkRequestHandlerImpl;
            friend class private_::AsyncRequestHandler;
            friend class NetworkRequestsHandler;
            explicit FriendClassesAPI(NetworkRequestBase& p_base) : m_base(p_base) {}

            NetworkRequestBase& m_base;

            /**
             * \brief Adds data to response data
             * \param p_data std::vector<uint8_t>&&
             */
            void addResponseData(std::vector< uint8_t >&& p_data);

            /**
             * \brief Sets current request status
             * \param p_status Status
             */
            void setStatus(Status p_status);

            /**
             * \brief Sets error
             * \param p_error_code std::error_code
             */
            void setError(std::error_code p_error_code);
        };

    public:

        friend bool operator<(const tristan::network::NetworkRequestBase& p_left, const tristan::network::NetworkRequestBase& p_right) {
            return p_left.m_priority < p_right.m_priority;
        }

        friend bool operator>(const tristan::network::NetworkRequestBase& p_left, const tristan::network::NetworkRequestBase& p_right) {
            return p_left.m_priority > p_right.m_priority;
        }

        /**
         * \brief Sets priority of the request
         * \param p_priority Priority
         */
        void setPriority(Priority p_priority);

        /**
         * \brief Sets total amount of bytes which should be read from socket
         * @param p_bytes uint64_t
         */
        void setBytesToRead(uint64_t p_bytes);

        /**
         * \brief Sets if request should be processed as Ssl. By default is set to false.
         * @param p_value bool
         */
        void setSSL(bool p_value = true);

        /**
         * Sets delimiter which will be used to truncate incoming stream.
         * @param p_delimiter std::vector< uint8_t >&&
         */
        void setResponseDelimiter(std::vector< uint8_t >&& p_delimiter);

        /**
         * Sets delimiter which will be used to truncate incoming stream.
         * @param p_delimiter const std::vector< uint8_t >&
         */
        void setResponseDelimiter(const std::vector< uint8_t >& p_delimiter);

        /**
         * \brief Sets file where response fom remote will be stored
         * \param p_path std::filesystem::path&&
         */
        void outputToFile(std::filesystem::path&& p_path);

        /**
         * \overload
         * \brief Sets file where response fom remote will be stored
         * \param p_path const std::filesystem::path&
         */
        void outputToFile(const std::filesystem::path& p_path);

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
         * \param p_request_data std::vector<uint8_t>&&
         */
        void setRequest(std::vector< uint8_t >&& p_request_data);

        /**
         * \brief Set request data using copy.
         * \param p_request_data const std::vector<uint8_t>&
         */
        void setRequest(const std::vector< uint8_t >& p_request_data);

        /**
         * \brief Sets timeout for socket operations
         * \param p_timeout std::chrono::seconds
         */
        void setTimeOut(std::chrono::seconds p_timeout);

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

        [[nodiscard]] auto timeout() const -> std::chrono::seconds;

        /**
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \param p_function std::function<void(uint64_t)>&&
         */
        void addReadBytesValueChangedCallback(std::function< void(uint64_t) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(uint64_t)
         */
        template < class Object > void addReadBytesValueChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(uint64_t));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(uint64_t)
         */
        template < class Object > void addReadBytesValueChangedCallback(Object* p_object, void (Object::*p_function)(uint64_t));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \param p_function std::function<void(uint64_t)>&&
         */
        void addReadBytesValueChangedCallback(std::function< void(const std::string&, uint64_t) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&, uint64_t)
         */
        template < class Object > void addReadBytesValueChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&, uint64_t));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked each time read bytes value is increased.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&, uint64_t)
         */
        template < class Object > void addReadBytesValueChangedCallback(Object* p_object, void (Object::*p_function)(const std::string&, uint64_t));

        /**
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param p_function std::function<void()>&& functor
         */
        void addFinishedCallback(std::function< void() >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addFinishedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addFinishedCallback(Object* p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param p_function std::function<void(const std::string&)>&& functor
         */
        void addFinishedCallback(std::function< void(const std::string&) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addFinishedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addFinishedCallback(Object* p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param p_function std::function<void(std::shared_ptr<std::vector<uint8_t>>)>&& functor
         */
        void addFinishedCallback(std::function< void(std::shared_ptr< NetworkResponse >) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object > void addFinishedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(std::shared_ptr< NetworkResponse >));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object > void addFinishedCallback(Object* p_object, void (Object::*p_function)(std::shared_ptr< NetworkResponse >));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \param p_function std::function<void(const std::string&, std::shared_ptr<std::vector<uint8_t>>)>&& functor
         */
        void addFinishedCallback(std::function< void(const std::string&, std::shared_ptr< NetworkResponse >) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&, std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object >
        void addFinishedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&, std::shared_ptr< NetworkResponse >));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request was processed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&, std::shared_ptr<std::vector<uint8_t>>)
         */
        template < class Object > void addFinishedCallback(Object* p_object, void (Object::*p_function)(const std::string&, std::shared_ptr< NetworkResponse >));

        /**
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \param p_function std::function<void()>&& functor
         */
        void addStatusChangedCallback(std::function< void() >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addStatusChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addStatusChangedCallback(Object* p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \param p_function std::function<void(const std::string&)>&& functor
         */
        void addStatusChangedCallback(std::function< void(const std::string&) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addStatusChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addStatusChangedCallback(Object* p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \param p_function std::function<void(Status)>&& functor
         */
        void addStatusChangedCallback(std::function< void(Status) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(Status)
         */
        template < class Object > void addStatusChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(Status));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(Status)
         */
        template < class Object > void addStatusChangedCallback(Object* p_object, void (Object::*p_function)(Status));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \param p_function std::function<void(const std::string&, Status)>&& functor
         */
        void addStatusChangedCallback(std::function< void(const std::string&, Status) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&, Status)
         */
        template < class Object > void addStatusChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&, Status));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request status had changed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&, Status)
         */
        template < class Object > void addStatusChangedCallback(Object* p_object, void (Object::*p_function)(const std::string&, Status));

        /**
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \param p_function std::function<void()>&&
         */
        void addPausedCallback(std::function< void() >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addPausedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addPausedCallback(Object* p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \param p_function std::function<void(const std::string&)>&&
         */
        void addPausedCallback(std::function< void(const std::string&) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addPausedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing is paused.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addPausedCallback(Object* p_object, void (Object::*p_function)(const std::string&));

        /**
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \param p_function std::function<void()>&&
         */
        void addResumedCallback(std::function< void() >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addResumedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addResumedCallback(Object* p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \param p_function std::function<void(const std::string&)>&&
         */
        void addResumedCallback(std::function< void(const std::string&) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addResumedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was resumed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addResumedCallback(Object* p_object, void (Object::*p_function)(const std::string&));

        /**
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \param p_function std::function<void()>&&
         */
        void addCanceledCallback(std::function< void() >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addCanceledCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addCanceledCallback(Object* p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \param p_function std::function<void(const std::string&)>&&
         */
        void addCanceledCallback(std::function< void(const std::string&) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addCanceledCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addCanceledCallback(Object* p_object, void (Object::*p_function)(const std::string&));

        /**
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \param p_function std::function<void()>&&
         */
        void addFailedCallback(std::function< void() >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addFailedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)()
         */
        template < class Object > void addFailedCallback(Object* p_object, void (Object::*p_function)());

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \param p_function std::function<void(const std::string&)>&&
         */
        void addFailedCallback(std::function< void(const std::string&) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addFailedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&)
         */
        template < class Object > void addFailedCallback(Object* p_object, void (Object::*p_function)(const std::string&));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \param p_function std::function<void(std::error_code)>&&
         */
        void addFailedCallback(std::function< void(std::error_code) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(std::error_code)
         */
        template < class Object > void addFailedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(std::error_code));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(std::error_code)
         */
        template < class Object > void addFailedCallback(Object* p_object, void (Object::*p_function)(std::error_code));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \param p_function std::function<void(const std::string&, std::error_code)>&&
         */
        void addFailedCallback(std::function< void(const std::string&, std::error_code) >&& p_function);

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing had failed.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object std::weak_ptr<Object>
         * \param p_function void (Object::*functor)(const std::string&, std::error_code)
         */
        template < class Object > void addFailedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&, std::error_code));

        /**
         * \overload
         * \brief Registers callback functions which will be invoked when network request processing was canceled.
         * \tparam Object Type which holds the function member to invoke
         * \param p_object Object*
         * \param p_function void (Object::*functor)(const std::string&, std::error_code)
         */
        template < class Object > void addFailedCallback(Object* p_object, void (Object::*p_function)(const std::string&, std::error_code));

        FriendClassesAPI request_handlers_api;

    protected:

        explicit NetworkRequestBase(Url&& p_url);

        virtual ~NetworkRequestBase() = default;

        void notifyWhenBytesReadChanged();

        void notifyWhenStatusChanged();

        void notifyWhenPaused();

        void notifyWhenResumed();

        void notifyWhenCanceled();

        void notifyWhenFinished();

        void notifyWhenFailed();

        /**
             * \brief Adds data to response data
             * \param p_data std::vector<uint8_t>&&
             */
        void addResponseData(std::vector< uint8_t >&& p_data);

        /**
             * \brief Sets current request status
             * \param p_status Status
             */
        void setStatus(Status p_status);

        /**
             * \brief Sets error
             * \param p_error_code std::error_code
             */
        void setError(std::error_code p_error_code);

        Url m_url;
        std::filesystem::path m_output_path;
        std::string m_uuid;
        std::vector< uint8_t > m_delimiter;
        std::vector< std::function< void(uint64_t) > > m_read_bytes_changed_callback_functors;
        std::vector< std::function< void(const std::string&, uint64_t) > > m_read_bytes_changed_with_id_callback_functors;
        std::vector< std::function< void() > > m_finished_void_callback_functors;
        std::vector< std::function< void(const std::string&) > > m_finished_with_id_callback_functors;
        std::vector< std::function< void(std::shared_ptr< NetworkResponse >) > > m_finished_with_response_callback_functors;
        std::vector< std::function< void(const std::string&, std::shared_ptr< NetworkResponse >) > > m_finished_with_id_and_response_callback_functors;
        std::vector< std::function< void() > > m_status_changed_void_callback_functors;
        std::vector< std::function< void(const std::string&) > > m_status_changed_with_id_callback_functors;
        std::vector< std::function< void(Status) > > m_status_changed_with_status_callback_functors;
        std::vector< std::function< void(const std::string&, Status) > > m_status_changed_with_id_and_status_callback_functors;
        std::vector< std::function< void() > > m_paused_void_functors;
        std::vector< std::function< void(const std::string&) > > m_paused_with_id_functors;
        std::vector< std::function< void() > > m_resumed_void_functors;
        std::vector< std::function< void(const std::string&) > > m_resumed_with_id_functors;
        std::vector< std::function< void() > > m_canceled_void_functors;
        std::vector< std::function< void(const std::string&) > > m_canceled_with_id_functors;
        std::vector< std::function< void() > > m_failed_void_callback_functors;
        std::vector< std::function< void(const std::string&) > > m_failed_with_id_callback_functors;
        std::vector< std::function< void(std::error_code) > > m_failed_with_error_code_callback_functors;
        std::vector< std::function< void(const std::string&, std::error_code) > > m_failed_with_id_and_error_code_callback_functors;
        std::vector< uint8_t > m_request_data;
        std::error_code m_error;
        std::shared_ptr< NetworkResponse > m_response;

        std::chrono::seconds m_timeout;

        uint64_t m_bytes_to_read;
        uint64_t m_bytes_read;
        std::unique_ptr< std::ofstream > m_output_file;

        Status m_status;
        Priority m_priority;
        std::atomic< bool > m_paused;
        std::atomic< bool > m_canceled;
        bool m_output_to_file;
        bool m_ssl;
    };

    template < class Object > void NetworkRequestBase::addReadBytesValueChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(uint64_t)) {
        m_read_bytes_changed_callback_functors.emplace_back([p_object, p_function](uint64_t value) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, value);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addReadBytesValueChangedCallback(Object* p_object, void (Object::*p_function)(uint64_t)) {
        m_read_bytes_changed_callback_functors.emplace_back([p_object, p_function](uint64_t value) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, value);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addReadBytesValueChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&, uint64_t)) {
        m_read_bytes_changed_callback_functors.emplace_back([p_object, p_function](const std::string& id, uint64_t value) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id, value);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addReadBytesValueChangedCallback(Object* p_object, void (Object::*p_function)(const std::string&, uint64_t)) {
        m_read_bytes_changed_callback_functors.emplace_back([p_object, p_function](const std::string& id, uint64_t value) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id, value);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFinishedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)()) {
        m_finished_void_callback_functors.emplace_back([p_object, p_function]() -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFinishedCallback(Object* p_object, void (Object::*p_function)()) {
        m_finished_void_callback_functors.emplace_back([p_object, p_function]() -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFinishedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&)) {
        m_finished_with_id_callback_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFinishedCallback(Object* p_object, void (Object::*p_function)(const std::string&)) {
        m_finished_with_id_callback_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFinishedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(std::shared_ptr< NetworkResponse >)) {
        m_finished_with_response_callback_functors.emplace_back([p_object, p_function](std::shared_ptr< std::vector< uint8_t > > response) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, response);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFinishedCallback(Object* p_object, void (Object::*p_function)(std::shared_ptr< NetworkResponse >)) {
        m_finished_with_id_callback_functors.emplace_back([p_object, p_function](std::shared_ptr< std::vector< uint8_t > > response) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, response);
            }
        });
    }

    template < class Object >
    void NetworkRequestBase::addFinishedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&, std::shared_ptr< NetworkResponse >)) {
        m_finished_with_response_callback_functors.emplace_back(
            [p_object, p_function](const std::string& id, std::shared_ptr< std::vector< uint8_t > > response) -> void {
                if (auto l_object = p_object.lock()) {
                    std::invoke(p_function, l_object, id, response);
                }
            });
    }

    template < class Object > void NetworkRequestBase::addFinishedCallback(Object* p_object, void (Object::*p_function)(const std::string&, std::shared_ptr< NetworkResponse >)) {
        m_finished_with_id_callback_functors.emplace_back(
            [p_object, p_function](const std::string& id, std::shared_ptr< std::vector< uint8_t > > response) -> void {
                if (p_object != nullptr) {
                    std::invoke(p_function, p_object, id, response);
                }
            });
    }

    template < class Object > void NetworkRequestBase::addStatusChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)()) {
        m_finished_void_callback_functors.emplace_back([p_object, p_function]() -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addStatusChangedCallback(Object* p_object, void (Object::*p_function)()) {
        m_finished_void_callback_functors.emplace_back([p_object, p_function]() -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addStatusChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&)) {
        m_finished_with_id_callback_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addStatusChangedCallback(Object* p_object, void (Object::*p_function)(const std::string&)) {
        m_finished_with_id_callback_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addStatusChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(Status)) {
        m_finished_with_response_callback_functors.emplace_back([p_object, p_function](Status status) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, status);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addStatusChangedCallback(Object* p_object, void (Object::*p_function)(Status)) {
        m_finished_with_id_callback_functors.emplace_back([p_object, p_function](Status status) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, status);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addStatusChangedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&, Status)) {
        m_finished_with_response_callback_functors.emplace_back([p_object, p_function](const std::string& id, Status status) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id, status);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addStatusChangedCallback(Object* p_object, void (Object::*p_function)(const std::string&, Status)) {
        m_finished_with_id_callback_functors.emplace_back([p_object, p_function](const std::string& id, Status status) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id, status);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addPausedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)()) {
        m_paused_void_functors.emplace_back([p_object, p_function]() -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addPausedCallback(Object* p_object, void (Object::*p_function)()) {
        m_paused_void_functors.emplace_back([p_object, p_function]() -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addPausedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&)) {
        m_paused_void_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addPausedCallback(Object* p_object, void (Object::*p_function)(const std::string&)) {
        m_paused_void_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addResumedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)()) {
        m_resumed_void_functors.emplace_back([p_object, p_function]() -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addResumedCallback(Object* p_object, void (Object::*p_function)()) {
        m_resumed_void_functors.emplace_back([p_object, p_function]() -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addResumedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&)) {
        m_resumed_with_id_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addResumedCallback(Object* p_object, void (Object::*p_function)(const std::string&)) {
        m_resumed_with_id_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addCanceledCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)()) {
        m_canceled_void_functors.emplace_back([p_object, p_function]() -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addCanceledCallback(Object* p_object, void (Object::*p_function)()) {
        m_canceled_void_functors.emplace_back([p_object, p_function]() -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addCanceledCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&)) {
        m_canceled_with_id_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addCanceledCallback(Object* p_object, void (Object::*p_function)(const std::string&)) {
        m_canceled_with_id_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFailedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)()) {
        m_failed_void_callback_functors.emplace_back([p_object, p_function]() -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFailedCallback(Object* p_object, void (Object::*p_function)()) {
        m_failed_void_callback_functors.emplace_back([p_object, p_function]() -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFailedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&)) {
        m_failed_with_id_callback_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFailedCallback(Object* p_object, void (Object::*p_function)(const std::string&)) {
        m_failed_with_id_callback_functors.emplace_back([p_object, p_function](const std::string& id) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFailedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(std::error_code)) {
        m_failed_with_error_code_callback_functors.emplace_back([p_object, p_function](std::error_code error_code) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, error_code);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFailedCallback(Object* p_object, void (Object::*p_function)(std::error_code)) {
        m_failed_with_error_code_callback_functors.emplace_back([p_object, p_function](std::error_code error_code) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, error_code);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFailedCallback(std::weak_ptr< Object > p_object, void (Object::*p_function)(const std::string&, std::error_code)) {
        m_failed_with_id_and_error_code_callback_functors.emplace_back([p_object, p_function](const std::string& id, std::error_code error_code) -> void {
            if (auto l_object = p_object.lock()) {
                std::invoke(p_function, l_object, id, error_code);
            }
        });
    }

    template < class Object > void NetworkRequestBase::addFailedCallback(Object* p_object, void (Object::*p_function)(const std::string&, std::error_code)) {
        m_failed_with_id_and_error_code_callback_functors.emplace_back([p_object, p_function](const std::string& id, std::error_code error_code) -> void {
            if (p_object != nullptr) {
                std::invoke(p_function, p_object, id, error_code);
            }
        });
    }

}  // namespace tristan::network

#endif  //NETWORK_REQUEST_BASE_HPP
