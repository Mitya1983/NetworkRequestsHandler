#ifndef DOWNLOADER_HPP
#define DOWNLOADER_HPP

#include "resumable_coroutine.hpp"

#include <memory>
#include <list>
#include <mutex>
#include <atomic>

// TODO: Implement some sort of hangup check

namespace asio {
    class io_context;
}

namespace tristan::network {

    class Downloader {
        Downloader();

    public:
        Downloader(const Downloader& other) = delete;
        Downloader(Downloader&& other) = delete;

        Downloader& operator=(const Downloader& other) = delete;
        Downloader& operator=(Downloader&& other) = delete;

        ~Downloader() = default;
        static auto create() -> std::unique_ptr< Downloader >;
        static auto run(const std::unique_ptr< Downloader >& downloader) -> std::error_code;

        void setMaxDownloadsCount(uint8_t count);
        void setWorking(bool value);

        template < class Socket >
        void addDownload(const Socket& socket, std::shared_ptr< NetworkRequest > network_request);

    protected:
    private:
        std::mutex m_downloads_lock;

        std::list< ResumableCoroutine > m_downloads;

        std::unique_ptr< asio::io_context > m_io_context;

        uint8_t m_max_downloads_count;
        std::atomic< bool > m_working;

        void _run();

        template < class Socket >
        auto doDownload(Socket& socket, std::shared_ptr< tristan::network::NetworkRequest > network_request)
            -> tristan::network::ResumableCoroutine;
    };
}  // namespace tristan::network

#endif  // DOWNLOADER_HPP
