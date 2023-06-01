#include "network_request_base.hpp"
#include "network_response.hpp"

#include <socket_error.hpp>

tristan::network::NetworkRequestBase::NetworkRequestBase(tristan::network::Url&& url) :
    request_handlers_api(*this),
    m_url(std::move(url)),
    m_uuid(utility::getUuid()),
    m_timeout(std::chrono::seconds(5)),
    m_bytes_to_read(0),
    m_bytes_read(0),
    m_status(Status::WAITING),
    m_priority(Priority::NORMAL),
    m_paused(false),
    m_canceled(false),
    m_output_to_file(false),
    m_ssl(false) { }

void tristan::network::NetworkRequestBase::notifyWhenBytesReadChanged() {
    if (not m_read_bytes_changed_callback_functors.empty()) {
        for (const auto& functor: m_read_bytes_changed_callback_functors) {
            functor(m_bytes_read);
        }
    }
    if (not m_read_bytes_changed_with_id_callback_functors.empty()) {
        for (const auto& functor: m_read_bytes_changed_with_id_callback_functors) {
            functor(m_uuid, m_bytes_read);
        }
    }
}

void tristan::network::NetworkRequestBase::notifyWhenStatusChanged() {
    if (not m_status_changed_void_callback_functors.empty()) {
        for (const auto& functor: m_status_changed_void_callback_functors) {
            functor();
        }
    }
    if (not m_status_changed_with_id_callback_functors.empty()) {
        for (const auto& functor: m_status_changed_with_id_callback_functors) {
            functor(m_uuid);
        }
    }
    if (not m_status_changed_with_status_callback_functors.empty()) {
        for (const auto& functor: m_status_changed_with_status_callback_functors) {
            functor(m_status);
        }
    }
    if (not m_status_changed_with_id_and_status_callback_functors.empty()) {
        for (const auto& functor: m_status_changed_with_id_and_status_callback_functors) {
            functor(m_uuid, m_status);
        }
    }
}

void tristan::network::NetworkRequestBase::notifyWhenPaused() {
    if (not m_paused_void_functors.empty()) {
        for (const auto& functor: m_paused_void_functors) {
            functor();
        }
    }
    if (not m_paused_with_id_functors.empty()) {
        for (const auto& functor: m_paused_with_id_functors) {
            functor(m_uuid);
        }
    }
}

void tristan::network::NetworkRequestBase::notifyWhenResumed() {
    if (not m_resumed_void_functors.empty()) {
        for (const auto& functor: m_resumed_void_functors) {
            functor();
        }
    }
    if (not m_resumed_with_id_functors.empty()) {
        for (const auto& functor: m_resumed_with_id_functors) {
            functor(m_uuid);
        }
    }
}

void tristan::network::NetworkRequestBase::notifyWhenCanceled() {
    if (not m_canceled_void_functors.empty()) {
        for (const auto& functor: m_canceled_void_functors) {
            functor();
        }
    }
    if (not m_canceled_with_id_functors.empty()) {
        for (const auto& functor: m_canceled_with_id_functors) {
            functor(m_uuid);
        }
    }
}

void tristan::network::NetworkRequestBase::notifyWhenFinished() {
    if (not m_finished_void_callback_functors.empty()) {
        for (const auto& functor: m_finished_void_callback_functors) {
            functor();
        }
    }
    if (not m_finished_with_id_callback_functors.empty()) {
        for (const auto& functor: m_finished_with_id_callback_functors) {
            functor(m_uuid);
        }
    }
    if (not m_finished_with_response_callback_functors.empty()) {
        for (const auto& functor: m_finished_with_response_callback_functors) {
            functor(m_response);
        }
    }
    if (not m_finished_with_id_and_response_callback_functors.empty()) {
        for (const auto& functor: m_finished_with_id_and_response_callback_functors) {
            functor(m_uuid, m_response);
        }
    }
}

void tristan::network::NetworkRequestBase::notifyWhenFailed() {
    if (not m_failed_void_callback_functors.empty()) {
        for (const auto& functor: m_failed_void_callback_functors) {
            functor();
        }
    }
    if (not m_failed_with_id_callback_functors.empty()) {
        for (const auto& functor: m_failed_with_id_callback_functors) {
            functor(m_uuid);
        }
    }
    if (not m_failed_with_error_code_callback_functors.empty()) {
        for (const auto& functor: m_failed_with_error_code_callback_functors) {
            functor(m_error);
        }
    }
    if (not m_failed_with_id_and_error_code_callback_functors.empty()) {
        for (const auto& functor: m_failed_with_id_and_error_code_callback_functors) {
            functor(m_uuid, m_error);
        }
    }
}

void tristan::network::NetworkRequestBase::addResponseData(std::vector< uint8_t >&& data) {
    auto data_size = data.size();
    if (not m_output_to_file) {
        if (not m_response) {
            m_response = tristan::network::NetworkResponse::createResponse(m_uuid);
            m_response->m_response_data = std::make_shared< std::vector< uint8_t > >(std::move(data));
        } else {
            if (not m_response->m_response_data){
                m_response->m_response_data = std::make_shared< std::vector< uint8_t > >(std::move(data));
            } else {
                m_response->m_response_data->insert(m_response->m_response_data->end(), data.begin(), data.end());
            }
        }
    } else {
        if (m_output_path.empty()) {
            tristan::network::NetworkRequestBase::setError(tristan::network::makeError(tristan::network::ErrorCode::FILE_PATH_EMPTY));
            return;
        }
        if (not m_output_file) {
            m_output_file = std::make_unique< std::ofstream >(m_output_path, std::ios::binary);
        }

        if (not m_output_file->is_open()) {
            m_output_file->open(m_output_path, std::ios::ate | std::ios::binary | std::ios::app);
            if (not m_output_file->is_open()) {
                tristan::network::NetworkRequestBase::setError(std::error_code(errno, std::system_category()));
                return;
            }
        }
        m_output_file->write(reinterpret_cast< const char* >(data.data()), static_cast< std::streamsize >(data.size()));
    }
    m_bytes_read += data_size;
    tristan::network::NetworkRequestBase::notifyWhenBytesReadChanged();
}

void tristan::network::NetworkRequestBase::setStatus(tristan::network::Status status) {
    tristan::network::NetworkRequestBase::notifyWhenStatusChanged();

    switch (status) {
        case tristan::network::Status::WAITING:
            [[fallthrough]];
        case tristan::network::Status::WRITING:
            [[fallthrough]];
        case tristan::network::Status::READING:
            [[fallthrough]];
        case tristan::network::Status::PROCESSED:
            break;
        case tristan::network::Status::PAUSED: {
            m_paused.store(true, std::memory_order_relaxed);
            tristan::network::NetworkRequestBase::notifyWhenPaused();
            if (m_output_file) {
                if (m_output_file->is_open()) {
                    m_output_file->close();
                }
                m_output_file.reset();
            }
            break;
        }
        case tristan::network::Status::RESUMED: {
            m_paused.store(false, std::memory_order_relaxed);
            tristan::network::NetworkRequestBase::notifyWhenResumed();
            break;
        }
        case tristan::network::Status::ERROR: {
            if (m_error.value() == static_cast< int >(tristan::sockets::Error::READ_DONE)) {
                return;
            }
            tristan::network::NetworkRequestBase::notifyWhenFailed();
            if (m_output_file) {
                if (m_output_file->is_open()) {
                    m_output_file->close();
                }
                m_output_file.reset();
            }
            if (std::filesystem::exists(m_output_path)) {
                std::filesystem::remove(m_output_path);
            }
            break;
        }
        case tristan::network::Status::CANCELED: {
            m_canceled.store(true, std::memory_order_relaxed);
            tristan::network::NetworkRequestBase::notifyWhenCanceled();
            if (m_output_file) {
                if (m_output_file->is_open()) {
                    m_output_file->close();
                }
                m_output_file.reset();
            }
            if (std::filesystem::exists(m_output_path)) {
                std::filesystem::remove(m_output_path);
            }
            break;
        }
        case tristan::network::Status::DONE: {
            tristan::network::NetworkRequestBase::notifyWhenFinished();
            if (m_output_file) {
                if (m_output_file->is_open()) {
                    m_output_file->close();
                }
                m_output_file.reset();
            }
            break;
        }
    }
    m_status = status;
}

void tristan::network::NetworkRequestBase::setError(std::error_code error_code) {
    m_error = error_code;
    if (m_error.value() != static_cast< int >(tristan::sockets::Error::READ_DONE)) {
        tristan::network::NetworkRequestBase::setStatus(tristan::network::Status::ERROR);
    }
}

void tristan::network::NetworkRequestBase::setPriority(tristan::network::Priority priority) { m_priority = priority; }

void tristan::network::NetworkRequestBase::setBytesToRead(uint64_t bytes) { m_bytes_to_read = bytes; }

void tristan::network::NetworkRequestBase::setSSL(bool value) { m_ssl = value; }

void tristan::network::NetworkRequestBase::setResponseDelimiter(std::vector< uint8_t >&& delimiter) { m_delimiter = std::move(delimiter); }

void tristan::network::NetworkRequestBase::setResponseDelimiter(const std::vector< uint8_t >& delimiter) { m_delimiter = delimiter; }

void tristan::network::NetworkRequestBase::outputToFile(std::filesystem::path&& path) {
    m_output_path = std::move(path);
    m_output_to_file = true;
}

void tristan::network::NetworkRequestBase::outputToFile(const std::filesystem::path& path) {
    m_output_path = path;
    m_output_to_file = true;
}

void tristan::network::NetworkRequestBase::cancel() { tristan::network::NetworkRequestBase::setStatus(tristan::network::Status::CANCELED); }

void tristan::network::NetworkRequestBase::pauseProcessing() { tristan::network::NetworkRequestBase::setStatus(tristan::network::Status::PAUSED); }

void tristan::network::NetworkRequestBase::continueProcessing() { tristan::network::NetworkRequestBase::setStatus(tristan::network::Status::RESUMED); }

void tristan::network::NetworkRequestBase::setRequest(std::vector< uint8_t >&& request_data) { m_request_data = std::move(request_data); }

void tristan::network::NetworkRequestBase::setRequest(const std::vector< uint8_t >& request_data) { m_request_data = request_data; }

void tristan::network::NetworkRequestBase::setTimeOut(std::chrono::seconds timeout) { m_timeout = timeout; }

auto tristan::network::NetworkRequestBase::uuid() const noexcept -> const std::string& { return m_uuid; }

auto tristan::network::NetworkRequestBase::url() const noexcept -> const tristan::network::Url& { return m_url; }

auto tristan::network::NetworkRequestBase::error() const noexcept -> const std::error_code& { return m_error; }

auto tristan::network::NetworkRequestBase::status() const noexcept -> tristan::network::Status { return m_status; }

auto tristan::network::NetworkRequestBase::isPaused() const noexcept -> bool { return m_paused; }

auto tristan::network::NetworkRequestBase::isCanceled() const noexcept -> bool { return m_canceled; }

auto tristan::network::NetworkRequestBase::priority() const noexcept -> tristan::network::Priority { return m_priority; }

auto tristan::network::NetworkRequestBase::bytesToRead() const noexcept -> uint64_t { return m_bytes_to_read; }

auto tristan::network::NetworkRequestBase::isSSL() const noexcept -> bool { return m_ssl; }

auto tristan::network::NetworkRequestBase::responseDelimiter() const noexcept -> const std::vector< uint8_t >& { return m_delimiter; }

//auto tristan::network::NetworkRequestBase::requestData() -> const std::vector< uint8_t >& { return m_request_data; }

auto tristan::network::NetworkRequestBase::response() -> std::shared_ptr< NetworkResponse > { return m_response; }

auto tristan::network::NetworkRequestBase::timeout() const -> std::chrono::seconds { return m_timeout; }

void tristan::network::NetworkRequestBase::addReadBytesValueChangedCallback(std::function< void(uint64_t) >&& functor) {
    m_read_bytes_changed_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addReadBytesValueChangedCallback(std::function< void(const std::string&, uint64_t) >&& functor) {
    m_read_bytes_changed_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addFinishedCallback(std::function< void() >&& functor) {
    m_finished_void_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addFinishedCallback(std::function< void(const std::string&) >&& functor) {
    m_finished_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addFinishedCallback(std::function< void(std::shared_ptr< NetworkResponse >) >&& functor) {
    m_finished_with_response_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addFinishedCallback(std::function< void(const std::string&, std::shared_ptr< NetworkResponse >) >&& functor) {
    m_finished_with_id_and_response_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addStatusChangedCallback(std::function< void() >&& functor) {
    m_status_changed_void_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addStatusChangedCallback(std::function< void(const std::string&) >&& functor) {
    m_status_changed_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addStatusChangedCallback(std::function< void(Status) >&& functor) {
    m_status_changed_with_status_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addStatusChangedCallback(std::function< void(const std::string&, Status) >&& functor) {
    m_status_changed_with_id_and_status_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addPausedCallback(std::function< void() >&& functor) { m_paused_void_functors.emplace_back(std::move(functor)); }

void tristan::network::NetworkRequestBase::addPausedCallback(std::function< void(const std::string&) >&& functor) {
    m_paused_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addResumedCallback(std::function< void() >&& functor) { m_resumed_void_functors.emplace_back(std::move(functor)); }

void tristan::network::NetworkRequestBase::addResumedCallback(std::function< void(const std::string&) >&& functor) {
    m_resumed_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addCanceledCallback(std::function< void() >&& functor) { m_canceled_void_functors.emplace_back(std::move(functor)); }

void tristan::network::NetworkRequestBase::addCanceledCallback(std::function< void(const std::string&) >&& functor) {
    m_canceled_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addFailedCallback(std::function< void() >&& functor) {
    m_failed_void_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addFailedCallback(std::function< void(const std::string&) >&& functor) {
    m_failed_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addFailedCallback(std::function< void(std::error_code) >&& functor) {
    m_failed_with_error_code_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::addFailedCallback(std::function< void(const std::string&, std::error_code) >&& functor) {
    m_failed_with_id_and_error_code_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestBase::FriendClassesAPI::addResponseData(std::vector< uint8_t >&& data) { m_base.addResponseData(std::move(data)); }

void tristan::network::NetworkRequestBase::FriendClassesAPI::setStatus(tristan::network::Status status) { m_base.setStatus(status); }

void tristan::network::NetworkRequestBase::FriendClassesAPI::setError(std::error_code error_code) { m_base.setError(error_code); }
