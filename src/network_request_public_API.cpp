#include "network_request_public_API.hpp"

tristan::network::NetworkRequestPublicAPI::NetworkRequestPublicAPI(tristan::network::Url&& url) :
    tristan::network::NetworkRequestBase(std::move(url)) {
    if (not m_url.isValid()) {
        tristan::network::NetworkRequestProtectedAPI::setError(tristan::network::makeError(tristan::network::ErrorCode::INVALID_URL));
    }
}

tristan::network::NetworkRequestPublicAPI::NetworkRequestPublicAPI(const tristan::network::Url& url) :
    tristan::network::NetworkRequestBase(tristan::network::Url(url)) { }

void tristan::network::NetworkRequestPublicAPI::setPriority(tristan::network::Priority priority) { m_priority = priority; }

void tristan::network::NetworkRequestPublicAPI::setBytesToRead(uint64_t bytes) { m_bytes_to_read = bytes; }

void tristan::network::NetworkRequestPublicAPI::setSSL(bool value) { m_ssl = value; }

void tristan::network::NetworkRequestPublicAPI::setResponseDelimiter(std::vector< uint8_t >&& delimiter) { m_delimiter = std::move(delimiter); }

void tristan::network::NetworkRequestPublicAPI::setResponseDelimiter(const std::vector< uint8_t >& delimiter) { m_delimiter = delimiter; }

void tristan::network::NetworkRequestPublicAPI::outputToFile(std::filesystem::path&& path) {
    m_output_path = std::move(path);
    m_output_to_file = true;
}

void tristan::network::NetworkRequestPublicAPI::outputToFile(const std::filesystem::path& path) {
    m_output_path = path;
    m_output_to_file = true;
}

void tristan::network::NetworkRequestPublicAPI::cancel() { tristan::network::NetworkRequestProtectedAPI::setStatus(tristan::network::Status::CANCELED); }

void tristan::network::NetworkRequestPublicAPI::pauseProcessing() { tristan::network::NetworkRequestProtectedAPI::setStatus(tristan::network::Status::PAUSED); }

void tristan::network::NetworkRequestPublicAPI::continueProcessing() {
    tristan::network::NetworkRequestProtectedAPI::setStatus(tristan::network::Status::RESUMED);
}

void tristan::network::NetworkRequestPublicAPI::setRequest(std::vector< uint8_t >&& request_data) { m_request_data = std::move(request_data); }

void tristan::network::NetworkRequestPublicAPI::setRequest(const std::vector< uint8_t >& request_data) { m_request_data = request_data; }

auto tristan::network::NetworkRequestPublicAPI::uuid() const noexcept -> const std::string& { return m_uuid; }

auto tristan::network::NetworkRequestPublicAPI::url() const noexcept -> const tristan::network::Url& { return m_url; }

auto tristan::network::NetworkRequestPublicAPI::error() const noexcept -> const std::error_code& { return m_error; }

auto tristan::network::NetworkRequestPublicAPI::status() const noexcept -> tristan::network::Status { return m_status; }

auto tristan::network::NetworkRequestPublicAPI::isPaused() const noexcept -> bool { return m_paused; }

auto tristan::network::NetworkRequestPublicAPI::isCanceled() const noexcept -> bool { return m_canceled; }

auto tristan::network::NetworkRequestPublicAPI::priority() const noexcept -> tristan::network::Priority { return m_priority; }

auto tristan::network::NetworkRequestPublicAPI::bytesToRead() const noexcept -> uint64_t { return m_bytes_to_read; }

auto tristan::network::NetworkRequestPublicAPI::isSSL() const noexcept -> bool { return m_ssl; }

auto tristan::network::NetworkRequestPublicAPI::responseDelimiter() const noexcept -> const std::vector< uint8_t >& { return m_delimiter; }

//auto tristan::network::NetworkRequestPublicAPI::requestData() -> const std::vector< uint8_t >& { return m_request_data; }

auto tristan::network::NetworkRequestPublicAPI::response() -> std::shared_ptr< NetworkResponse > { return m_response; }

void tristan::network::NetworkRequestPublicAPI::addReadBytesValueChangedCallback(std::function< void(uint64_t) >&& functor) {
    m_read_bytes_changed_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addReadBytesValueChangedCallback(std::function< void(const std::string&, uint64_t) >&& functor) {
    m_read_bytes_changed_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addFinishedCallback(std::function< void() >&& functor) {
    m_finished_void_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addFinishedCallback(std::function< void(const std::string&) >&& functor) {
    m_finished_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addFinishedCallback(std::function< void(std::shared_ptr< NetworkResponse >) >&& functor) {
    m_finished_with_response_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addFinishedCallback(std::function< void(const std::string&, std::shared_ptr< NetworkResponse >) >&& functor) {
    m_finished_with_id_and_response_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addStatusChangedCallback(std::function< void() >&& functor) {
    m_status_changed_void_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addStatusChangedCallback(std::function< void(const std::string&) >&& functor) {
    m_status_changed_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addStatusChangedCallback(std::function< void(Status) >&& functor) {
    m_status_changed_with_status_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addStatusChangedCallback(std::function< void(const std::string&, Status) >&& functor) {
    m_status_changed_with_id_and_status_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addPausedCallback(std::function< void() >&& functor) {
    m_paused_void_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addPausedCallback(std::function< void(const std::string&) >&& functor) {
    m_paused_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addResumedCallback(std::function< void() >&& functor) {
    m_resumed_void_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addResumedCallback(std::function< void(const std::string&) >&& functor) {
    m_resumed_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addCanceledCallback(std::function< void() >&& functor) {
    m_canceled_void_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addCanceledCallback(std::function< void(const std::string&) >&& functor) {
    m_canceled_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addFailedCallback(std::function< void() >&& functor) {
    m_failed_void_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addFailedCallback(std::function< void(const std::string&) >&& functor) {
    m_failed_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addFailedCallback(std::function< void(std::error_code) >&& functor) {
    m_failed_with_error_code_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequestPublicAPI::addFailedCallback(std::function< void(const std::string&, std::error_code) >&& functor) {
    m_failed_with_id_and_error_code_callback_functors.emplace_back(std::move(functor));
}
