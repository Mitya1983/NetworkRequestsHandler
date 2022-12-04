#include "network_request_private_API.hpp"

void tristan::network::NetworkRequestPrivateAPI::notifyWhenBytesReadChanged() {
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

void tristan::network::NetworkRequestPrivateAPI::notifyWhenStatusChanged() {
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

void tristan::network::NetworkRequestPrivateAPI::notifyWhenPaused() {
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

void tristan::network::NetworkRequestPrivateAPI::notifyWhenResumed() {
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

void tristan::network::NetworkRequestPrivateAPI::notifyWhenCanceled() {
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

void tristan::network::NetworkRequestPrivateAPI::notifyWhenFinished() {
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

void tristan::network::NetworkRequestPrivateAPI::notifyWhenFailed() {
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
