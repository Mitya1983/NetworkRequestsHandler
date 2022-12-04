#ifndef NETWORK_REQUEST_BASE_HPP
#define NETWORK_REQUEST_BASE_HPP

#include "url.hpp"
#include "network_utility.hpp"

#include <filesystem>
#include <vector>
#include <functional>
#include <atomic>
#include <fstream>

namespace tristan::network {

    class NetworkResponse;

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
    protected:

        NetworkRequestBase() :
            m_bytes_to_read(0),
            m_bytes_read(0),
            m_status(Status::WAITING),
            m_priority(Priority::NORMAL),
            m_paused(false),
            m_canceled(false),
            m_output_to_file(false),
            m_ssl(false)
        {}

        explicit NetworkRequestBase(Url&& url) :
            m_url(std::move(url)),
            m_uuid(utility::getUuid()),
            m_delimiter(),
            m_bytes_to_read(0),
            m_bytes_read(0),
            m_status(Status::WAITING),
            m_priority(Priority::NORMAL),
            m_paused(false),
            m_canceled(false),
            m_output_to_file(false),
            m_ssl(false) { }

        virtual ~NetworkRequestBase() = default;

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

}  // namespace tristan::network

#endif  //NETWORK_REQUEST_BASE_HPP
