#include "network_logger.hpp"

tristan::network::Logger::Logger() :
    m_logger(tristan::log::Log::createLogInstance()) {
    m_logger->setModuleName("Network");
}

void tristan::network::Logger::setLogger(std::unique_ptr< tristan::log::Log >&& p_log) {
    tristan::network::Logger::instance().m_logger = std::move(p_log);
}

void tristan::network::Logger::write(tristan::log::LogEvent&& event) { tristan::network::Logger::instance().m_logger->write(std::move(event)); }

auto tristan::network::Logger::instance() -> tristan::network::Logger& {
    static tristan::network::Logger logger;

    return logger;
}
