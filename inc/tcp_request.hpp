#ifndef NETWORK_REQUEST_HPP
#define NETWORK_REQUEST_HPP

#include "network_request_base.hpp"
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
     * \class TcpRequest
     * \brief Used as a base class for network request classes. E.g. HttpRequest.
     */
    class TcpRequest : virtual public NetworkRequestBase {

    public:

        /**
         * \brief Constructor
         * \param p_url Uri&&
         */
        explicit TcpRequest(Url&& p_url);

        /**
         * \overload
         * \brief Constructor
         * \param uri const Url&
         */
        explicit TcpRequest(const Url& p_url);

        TcpRequest() = delete;

        TcpRequest(const TcpRequest& p_other) = delete;

        TcpRequest(TcpRequest&& p_other) noexcept = delete;

        TcpRequest& operator=(const TcpRequest& p_other) = delete;

        TcpRequest& operator=(TcpRequest&& p_other) noexcept = delete;

        ~TcpRequest() override = default;

        auto requestData() -> const std::vector<uint8_t>& override;

    };

}  // namespace tristan::network

#endif  // NETWORK_REQUEST_HPP
