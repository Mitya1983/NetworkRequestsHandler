#ifndef NETWORK_REQUEST_PROTECTED_API_HPP
#define NETWORK_REQUEST_PROTECTED_API_HPP

#include "network_request_private_API.hpp"

namespace tristan::network {

    class NetworkRequestProtectedAPI : virtual protected NetworkRequestPrivateAPI {
    protected:
        ~NetworkRequestProtectedAPI() override = default;

        friend class NetworkRequestsHandler;
        friend class AsyncRequestHandler;
        friend class RequestHandler;
        /**
             * \brief Adds data to response data
             * \param network_request NetworkRequest&
             * \param data std::vector<uint8_t>&&
             */
        void addResponseData(std::vector< uint8_t >&& data);

        /**
             * \brief Sets current request status
             * \param network_request NetworkRequest&
             * \param status Status
             */
        void setStatus(Status status);

        /**
             * \brief Sets error
             * \param network_request NetworkRequest&
             * \param error_code std::error_code
             */
        void setError(std::error_code error_code);
    };

}  // namespace tristan::network

#endif  //NETWORK_REQUEST_PROTECTED_API_HPP
