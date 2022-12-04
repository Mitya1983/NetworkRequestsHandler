#ifndef NETWORK_REQUEST_HPP
#define NETWORK_REQUEST_HPP

#include "network_request_public_API.hpp"
#include "network_utility.hpp"
#include "network_response.hpp"
#include "network_error.hpp"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <concepts>

namespace tristan::network {

    /**
     * \class NetworkRequest
     * \brief Used as a base class for network request classes. E.g. HttpRequest.
     */
    class NetworkRequest : public NetworkRequestPublicAPI {

    public:

        /**
         * \brief Constructor
         * \param url Uri&&
         */
        explicit NetworkRequest(Url&& url);

        /**
         * \overload
         * \brief Constructor
         * \param uri const Url&
         */
        explicit NetworkRequest(const Url& url);

        NetworkRequest() = delete;

        NetworkRequest(const NetworkRequest& other) = delete;

        NetworkRequest(NetworkRequest&& other) noexcept = delete;

        NetworkRequest& operator=(const NetworkRequest& other) = delete;

        NetworkRequest& operator=(NetworkRequest&& other) noexcept = delete;

        ~NetworkRequest() override = default;

        auto requestData() -> const std::vector<uint8_t>& override;

    };

}  // namespace tristan::network

#endif  // NETWORK_REQUEST_HPP
