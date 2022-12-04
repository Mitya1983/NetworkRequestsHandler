#ifndef NETWORK_REQUEST_PRIVATE_API_HPP
#define NETWORK_REQUEST_PRIVATE_API_HPP

#include "network_request_base.hpp"

namespace tristan::network {

    class NetworkRequestPrivateAPI : virtual protected NetworkRequestBase{

    protected:

        ~NetworkRequestPrivateAPI() override = default;

        void notifyWhenBytesReadChanged();

        void notifyWhenStatusChanged();

        void notifyWhenPaused();

        void notifyWhenResumed();

        void notifyWhenCanceled();

        void notifyWhenFinished();

        void notifyWhenFailed();
    };

} //End of tristan::network namespace

#endif  //NETWORK_REQUEST_PRIVATE_API_HPP
