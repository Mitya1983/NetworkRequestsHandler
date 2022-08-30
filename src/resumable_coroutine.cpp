#include "resumable_coroutine.hpp"

tristan::network::ResumableCoroutine::ResumableCoroutine(tristan::network::coroutine_handle handle) :
        m_handle(handle) {

}

bool tristan::network::ResumableCoroutine::resume() {
    if (not m_handle.done()){
        m_handle.resume();
    }
    return not m_handle.done();
}