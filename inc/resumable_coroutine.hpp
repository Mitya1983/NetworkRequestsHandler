#ifndef RESUMABLE_COROUTINE_HPP
#define RESUMABLE_COROUTINE_HPP
#include <exception>
#include <coroutine>

namespace tristan::network {

    class ResumableCoroutine {
    public:
        struct promise_type;
        using coroutine_handle = std::coroutine_handle< promise_type >;
        ResumableCoroutine(coroutine_handle handle);

        ResumableCoroutine(const ResumableCoroutine &other) = default;

        ResumableCoroutine(ResumableCoroutine &&other) = default;

        ResumableCoroutine &operator=(const ResumableCoroutine &other) = default;

        ResumableCoroutine &operator=(ResumableCoroutine &&other) = default;

        ~ResumableCoroutine() = default;

        bool resume();

    protected:

    private:
        coroutine_handle m_handle;
    };

    struct ResumableCoroutine::promise_type {
        using coroutine_handle = std::coroutine_handle<promise_type>;

        auto get_return_object() {
            return coroutine_handle::from_promise(*this);
        }

        auto initial_suspend() { return std::suspend_always(); }

        auto final_suspend() noexcept { return std::suspend_always(); }

        void return_void() {}

        void unhandled_exception() {
            std::terminate();
        }
    };

} //End of tristan::network namespace

#endif //RESUMABLE_COROUTINE_HPP
