#include "network_request_protected_API.hpp"

#include "network_response.hpp"

void tristan::network::NetworkRequestProtectedAPI::addResponseData(std::vector< uint8_t >&& data) {
    if (not m_output_to_file) {
        if (not m_response) {
            m_response = tristan::network::NetworkResponse::createResponse(m_uuid);
            m_response->m_response_data = std::make_shared< std::vector< uint8_t > >(std::move(data));
        } else {
            m_response->m_response_data->insert(m_response->m_response_data->end(), data.begin(), data.end());
        }
    } else {
        if (m_output_path.empty()) {
            tristan::network::NetworkRequestProtectedAPI::setError(tristan::network::makeError(tristan::network::ErrorCode::FILE_PATH_EMPTY));
            return;
        }
        if (not std::filesystem::exists(m_output_path)) {
            if (not std::filesystem::exists(m_output_path.parent_path())) {
                tristan::network::NetworkRequestProtectedAPI::setError(tristan::network::makeError(tristan::network::ErrorCode::DESTINATION_DIR_DOES_NOT_EXISTS));
                return;
            }
        }

        if (not m_output_file){
            m_output_file = std::unique_ptr<std::ofstream>();
        }

        if (not m_output_file->is_open()) {
            m_output_file->open(m_output_path, std::ios::ate | std::ios::binary | std::ios::app);
            if (not m_output_file->is_open()) {
                tristan::network::NetworkRequestProtectedAPI::setError(std::error_code(errno, std::system_category()));
            }
        }
        m_output_file->write(reinterpret_cast< const char* >(data.data()), static_cast< std::streamsize >(data.size()));
    }
    m_bytes_read = m_response->m_response_data->size();
}

void tristan::network::NetworkRequestProtectedAPI::setStatus(tristan::network::Status status) {
    tristan::network::NetworkRequestPrivateAPI::notifyWhenStatusChanged();

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
            tristan::network::NetworkRequestPrivateAPI::notifyWhenPaused();
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
            tristan::network::NetworkRequestPrivateAPI::notifyWhenResumed();
            break;
        }
        case tristan::network::Status::ERROR: {
            tristan::network::NetworkRequestPrivateAPI::notifyWhenFailed();
            if (m_output_file) {
                if (m_output_file->is_open()) {
                    m_output_file->close();
                }
                m_output_file.reset();
            }
            if (std::filesystem::exists(m_output_path)) {
                std::filesystem::remove(m_output_path);
            }
        }
        case tristan::network::Status::CANCELED: {
            m_canceled.store(true, std::memory_order_relaxed);
            tristan::network::NetworkRequestPrivateAPI::notifyWhenCanceled();
            if (m_output_file) {
                if (m_output_file->is_open()) {
                    m_output_file->close();
                }
                m_output_file.reset();
            }
            if (std::filesystem::exists(m_output_path)) {
                std::filesystem::remove(m_output_path);
            }
        }
        case tristan::network::Status::DONE: {
            tristan::network::NetworkRequestPrivateAPI::notifyWhenFinished();
            if (m_output_file) {
                if (m_output_file->is_open()) {
                    m_output_file->close();
                }
                m_output_file.reset();
            }
        }
    }
    m_status = status;
}

void tristan::network::NetworkRequestProtectedAPI::setError(std::error_code error_code) {
    m_error = error_code;
    tristan::network::NetworkRequestProtectedAPI::setStatus(tristan::network::Status::ERROR);
}
