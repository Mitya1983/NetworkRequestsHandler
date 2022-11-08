#include "downloader.hpp"
#include "network_error.hpp"

#include "asio/io_context.hpp"
#include "asio/connect.hpp"
#include "asio/read.hpp"
#include "asio/ssl/error.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/ssl/stream.hpp"

#include <thread>
#include <limits>
#include <vector>

namespace {

    constexpr uint16_t g_max_frame = std::numeric_limits< uint16_t >::max();

}  // End of anonymous namespace

tristan::network::Downloader::Downloader() :
    m_max_downloads_count(10),
    m_io_context(std::make_unique< asio::io_context >()) { }

auto tristan::network::Downloader::create() -> std::unique_ptr< Downloader > {
    return std::unique_ptr< Downloader >(new Downloader());
}

void tristan::network::Downloader::setMaxDownloadsCount(uint8_t count) { m_max_downloads_count = count; }

void tristan::network::Downloader::setWorking(bool value) {
    m_working.store(value, std::memory_order_relaxed);
}

auto tristan::network::Downloader::run(const std::unique_ptr< Downloader >& downloader) -> std::error_code {
    if (downloader->m_working.load(std::memory_order_relaxed)) {
        return tristan::network::makeError(tristan::network::ErrorCode::DOWNLOADER_LUNCHED_TWICE);
    }

    try {
        std::thread(&tristan::network::Downloader::_run, std::ref(*downloader)).detach();
        return {};
    } catch (const std::system_error& er) { return er.code(); }
}

void tristan::network::Downloader::_run() {

    if (m_downloads.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    } else {
        uint8_t active_downloads_counter = 0;
        while (m_working.load(std::memory_order_relaxed)) {
            for (auto active_download_iterator = m_downloads.begin();
                 active_download_iterator != m_downloads.end();) {
                if (active_downloads_counter > m_max_downloads_count) {
                    continue;
                }
                if (not active_download_iterator->resume()) {
                    std::scoped_lock< std::mutex > lock(m_downloads_lock);
                    active_download_iterator = m_downloads.erase(active_download_iterator);
                    --active_downloads_counter;
                } else {
                    ++active_download_iterator;
                    ++active_downloads_counter;
                }
            }
        }
    }
}

template < class Socket >
auto tristan::network::Downloader::doDownload(
    Socket& socket,
    std::shared_ptr< tristan::network::NetworkRequest > network_request)
    -> tristan::network::ResumableCoroutine {
    Socket l_socket(m_io_context);
    l_socket.assign(socket.remote_endpoint(), socket.release());

    if (network_request->status() != tristan::network::Status::DOWNLOADING) {
        tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request,
                                                                       tristan::network::Status::DOWNLOADING);
    }

    uint64_t bytes_read = 0;
    uint64_t bytes_to_read = network_request->bytesToRead();
    while (bytes_read < bytes_to_read) {
        std::vector< uint8_t > data;
        auto bytes_remain = bytes_to_read - bytes_read;
        uint16_t current_frame_size = (g_max_frame < bytes_remain ? g_max_frame : bytes_remain);
        data.reserve(current_frame_size);

        std::error_code error_code;
        asio::read(
            socket, asio::dynamic_buffer(data), asio::transfer_exactly(current_frame_size), error_code);
        if (error_code && error_code != asio::error::eof
            && error_code != asio::ssl::error::stream_truncated) {
            tristan::network::NetworkRequest::ProtectedMembers::pSetError(network_request, error_code);
            break;
        }
        bytes_read += current_frame_size;
        tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(network_request,
                                                                             std::move(data));
        co_await std::suspend_always();
    }
    tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(network_request,
                                                                   tristan::network::Status::DONE);
}
