#ifndef STATUS_CODES_HPP
#define STATUS_CODES_HPP

#include <cstdint>
#include <string>

namespace network {


enum class HttpStatus : uint8_t {
    Continue,
    Switching_Protocol,
    Processing,
    Early_Hints,
    Ok,
    Created,
    Accepted,
    Non_Authoritative_Information,
    No_Content,
    Reset_Content,
    Parial_Content,
    Multi_Status,
    Already_Reported,
    IM_Used,
    Multiple_Choice,
    Moved_Permanently,
    Found,
    See_Other,
    Not_Modified,
    Use_Proxy,
    Switch_Proxy,
    Temporary_Redirect,
    Permanent_Redirect,
    Bad_Request,
    Unauthorized,
    Payment_Required,
    Forbidden,
    Not_Found,
    Method_Not_Allowed,
    Not_Acceptable,
    Proxy_Authentication_Required,
    Request_Timeout,
    Conflict,
    Gone,
    Length_Required,
    Precondition_Failed,
    Request_Entity_Too_Large,
    Request_URI_Too_Long,
    Unsupported_Media_Type,
    Requested_Range_Not_Satisfiable,
    Expectation_Failed,
    Misdirected_Request,
    Unprocessable_Entity,
    Locked,
    Failed_Dependency,
    Too_Early,
    Upgrade_Required,
    Precondition_Required,
    Too_Many_Requests,
    Request_Header_Fields_Too_Large,
    Unavailable_For_Legal_Reasons,
    Internal_Server_Error,
    Not_Implemented,
    Bad_Gateway,
    Service_Unavailable,
    Gateway_Timeout,
    HTTP_Version_Not_Supported,
    Variant_Also_Negotiates,
    Insufficient_Storage,
    Loop_Detected,
    Not_Extended,
    Network_Authentication_Required
};

std::pair<std::string, std::string> getHttpStatusDetails(HttpStatus status);
} // namespace network
#endif // STATUS_CODES_HPP
