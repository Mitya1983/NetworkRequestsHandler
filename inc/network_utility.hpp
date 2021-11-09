#ifndef NETWORK_UTILITY_HPP
#define NETWORK_UTILITY_HPP

#include <filesystem>

namespace tristan::network::utility{
    /**
     * \brief Checks if filename exist and modifies the names by addis (1, 2, 3, ..., n) just before the las dot.
     * \param path std::filesystem::path&
     */
    void checkFileName(std::filesystem::path& path);
    /**
     * \brief Generates UUID Version 4.
     * \return UUID as a std::string.
     */
    [[nodiscard]] auto getUUID() -> std::string;
} //End of tristan::network::utility namespace

#endif //NETWORK_UTILITY_HPP
