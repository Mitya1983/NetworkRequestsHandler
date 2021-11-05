#ifndef NETWORK_UTILITY_HPP
#define NETWORK_UTILITY_HPP

#include <filesystem>

namespace tristan::network::utility{

    void checkFileName(std::filesystem::path& path);
    [[nodiscard]] auto getUUID() -> std::string;
} //End of tristan::network::utility namespace

#endif //NETWORK_UTILITY_HPP
