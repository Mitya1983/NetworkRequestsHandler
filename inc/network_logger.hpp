#ifndef NETWORK_LOGGER_HPP
#define NETWORK_LOGGER_HPP

#include <log.hpp>

#include <memory>

namespace tristan::network {

    class Logger {
    public:
        Logger(const Logger& other) = delete;
        Logger(Logger&& other) = delete;
        Logger& operator=(const Logger& other) = delete;

        Logger& operator=(Logger&& other) = delete;
        ~Logger() = default;

        static void write(tristan::log::LogEvent&& event);

    protected:
    private:
        Logger();
        static auto instance() -> Logger&;
        std::unique_ptr< tristan::log::Log > m_logger;
    };

#if defined(LOG_DISABLE_TRACE)
  #define netTrace
#else
  #define netTrace(message)                                 \
    tristan::network::Logger::write(tristan::log::LogEvent( \
        message, tristan::log::MessageType::Trace, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif
#if defined(LOG_DISABLE_DEBUG)
  #define netDebug
#else
  #define netDebug(message)                                 \
    tristan::network::Logger::write(tristan::log::LogEvent( \
        message, tristan::log::MessageType::Debug, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif
#if defined(LOG_DISABLE_ERROR)
  #define netError
#else
  #define netError(message)                                 \
    tristan::network::Logger::write(tristan::log::LogEvent( \
        message, tristan::log::MessageType::Error, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif
#if defined(LOG_DISABLE_WARNING)
  #define netWarning
#else
  #define netWarning(message)                               \
    tristan::network::Logger::write(tristan::log::LogEvent( \
        message, tristan::log::MessageType::Warning, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif
#if defined(LOG_DISABLE_INFO)
  #define netInfo
#else
  #define netInfo(message)                                  \
    tristan::network::Logger::write(tristan::log::LogEvent( \
        message, tristan::log::MessageType::Info, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif
#if defined(LOG_DISABLE_FATAL)
  #define netFatal
#else
  #define netFatal(message)                                 \
    tristan::network::Logger::write(tristan::log::LogEvent( \
        message, tristan::log::MessageType::Fatal, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif

}  // namespace tristan::network

#endif  //NETWORK_LOGGER_HPP
