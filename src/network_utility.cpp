#include "network_utility.hpp"
#include "net_log.hpp"
#include <arpa/inet.h>

#include <cmath>
#include <random>
#include <unordered_map>

namespace {

    std::unordered_map< char, std::string > percentage_encoding = {
        {' ',  "%20"},
        {'!',  "%21"},
        {'@',  "%40"},
        {'#',  "%23"},
        {'$',  "%24"},
        {'%',  "%25"},
        {'&',  "%26"},
        {'*',  "%2A"},
        {'(',  "%28"},
        {')',  "%29"},
        {'+',  "%2B"},
        {'=',  "%3D"},
        {'[',  "%5B"},
        {']',  "%5D"},
        {':',  "%3A"},
        {';',  "%3B"},
        {'\'', "%27"},
        {',',  "%2C"},
        {'/',  "%2F"},
        {'?',  "%3F"},
    };

}  //End of unnamed namespace

void tristan::network::utility::checkFileName(std::filesystem::path& path) {
    netDebug("Checking filename " + path.string());
    uint16_t file_counter = 1;
    while (true) {
        if (std::filesystem::exists(path)) {
            auto file_extension = path.extension().string();
            if (not file_extension.empty()) {
                path.replace_extension("");
            }
            auto file_name = path.filename().string();
            if (file_counter == 1) {
                file_name += "(" + std::to_string(file_counter) + ")";
            } else {
                auto pos = file_name.find("(" + std::to_string(file_counter - 1) + ")");
                auto replace_counter = static_cast< uint16_t >(log10(file_counter) + 1);
                if (file_counter == 10 || file_counter == 100 || file_counter == 1000
                    || file_counter == 10000) {
                    file_name.replace(pos + 1, replace_counter - 1, std::to_string(file_counter));
                } else {
                    file_name.replace(pos + 1, replace_counter, std::to_string(file_counter));
                }
            }
            path.replace_filename(file_name);
            if (not file_extension.empty()) {
                path.replace_extension(file_extension);
            }
            ++file_counter;
        } else {
            break;
        }
    }
}

auto tristan::network::utility::getUuid() -> std::string {
    netDebug("Generating new UUID");
    std::string uuid;

    std::uniform_int_distribution< uint8_t > distribution_one(48, 57);
    std::uniform_int_distribution< uint8_t > distribution_two(97, 102);
    std::uniform_int_distribution< uint8_t > distribution_choice(1, 2);
    for (int i = 0; i < 36; ++i) {
        std::mt19937_64 generator(std::chrono::system_clock::now().time_since_epoch().count());
        uint8_t which_distribution = distribution_choice(generator);
        uint8_t uuid_value = 0;
        switch (which_distribution) {
            case 1: {
                uuid_value = distribution_one(generator);
                break;
            }
            case 2: {
                uuid_value = distribution_two(generator);
                break;
            }
            default: {
                break;
            }
        }
        uuid += static_cast< char >(uuid_value);
        if (uuid.size() == 8 || uuid.size() == 13 || uuid.size() == 18 || uuid.size() == 23) {
            uuid += '-';
        }
        if (uuid.size() == 14) {
            uuid += '4';
        }
        if (uuid.size() == 19) {
            uuid += 'a';
        }
    }

    return uuid;
}

auto tristan::network::utility::encodeUrl(const std::string& string_to_encode) -> std::string {
    netDebug("Encoding url " + string_to_encode);
    std::string result(string_to_encode);
    size_t char_to_encode = 0;
    while (true) {
        char_to_encode = result.find_first_of(" !@#$%&*()+=[]:;\',/?", char_to_encode);
        if (char_to_encode == std::string::npos) {
            break;
        }
        result.replace(char_to_encode, 1, percentage_encoding.at(result.at(char_to_encode)));
        ++char_to_encode;
    }
    netDebug("Encoded url is " + result);
    return result;
}

auto tristan::network::utility::uint32_tIpToStringIp(uint32_t ip) -> std::string {
    netDebug("Converting uint32_t ip representation" + std::to_string(ip) + " to string representation");
    char buf[INET_ADDRSTRLEN];
    std::string address = inet_ntop(AF_INET, &ip, buf, sizeof(buf));
    if (address.empty()){
        netError("Failed to convert ip address");
        return {};
    }
    netDebug("Converted ip is " + address);
    return address;
}

auto tristan::network::utility::stringIpToUint32_tIp(const std::string& ip) -> uint32_t {
    netDebug("Converting string ip representation" + ip + " to uint32_t representation");
    uint32_t result;
    if (inet_pton(AF_INET, ip.c_str(), &result) <= 0){
        netError("Failed to convert ip address");
        return {};
    }
    netDebug("Converted ip is " + std::to_string(result));
    return result;
}
