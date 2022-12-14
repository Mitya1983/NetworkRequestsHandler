#ifndef HTTP_STATUS_CODES_HPP
#define HTTP_STATUS_CODES_HPP

#include <cstdint>
#include <string>

namespace tristan::network{
    /**
     * \enum HttpStatus
     * \brief Stores values for http error codes.
     */
    enum class HttpStatus : uint16_t{
        ///The server has received the request headers and the client should proceed to send the request body.
        Continue = 100,
        ///The requester has asked the server to switch protocols and the server has agreed to do so.
        Switching_Protocol = 101,
        ///This code indicates that the server has received and is processing the request, but no response is available yet.
        Processing = 102,
        ///Used to return some response headers before final HTTP message.
        Early_Hints = 103,
        ///Standard response for successful HTTP requests.
        Ok = 200,
        ///The request has been fulfilled, resulting in the creation of a new resource.
        Created = 201,
        ///The request has been accepted for processing, but the processing has not been completed. The request might or might not be eventually acted upon, and may be disallowed when processing occurs.
        Accepted = 202,
        ///The server is a transforming proxy (e.g. a Web accelerator) that received a 200 OK from its origin, but is returning a modified version of the origin's response.
        Non_Authoritative_Information = 203,
        ///The server successfully processed the request, and is not returning any content.
        No_Content = 204,
        ///The server successfully processed the request, asks that the requester reset its document view, and is not returning any content.
        Reset_Content = 205,
        ///The server is delivering only part of the resource (byte serving) due to a range header sent by the client. The range header is used by HTTP clients to enable resuming of interrupted downloads, or split a download into multiple simultaneous streams.
        Partial_Content = 206,
        ///The message body that follows is by default an XML message and can contain a number of separate response codes, depending on how many sub-requests were made.
        Multi_Status = 207,
        ///The members of a DAV binding have already been enumerated in a preceding part of the (multistatus) response, and are not being included again.
        Already_Reported = 208,
        ///The server has fulfilled a request for the resource, and the response is a representation of the result of one or more instance-manipulations applied to the current instance.
        IM_Used = 226,
        ///Indicates multiple options for the resource from which the client may choose.
        Multiple_Choice = 300,
        ///This and all future requests should be directed to the given URI.
        Moved_Permanently = 301,
        ///Tells the client to look at (browse to) another URL. 302 has been superseded by 303 and 307.
        Found = 302,
        ///The response to the request can be found under another URI using the GET method. When received in response to a POST (or PUT/DELETE), the client should presume that the server has received the data and should issue a new GET request to the given URI.
        See_Other = 303,
        ///Indicates that the resource has not been modified since the version specified by the request headers.
        Not_Modified = 304,
        ///The requested resource is available only through a proxy, the address for which is provided in the response.
        Use_Proxy = 305,
        ///No longer used. Originally meant "Subsequent requests should use the specified proxy."
        Switch_Proxy = 306,
        ///In this case, the request should be repeated with another URI; however, future requests should still use the original URI. In contrast to how 302 was historically implemented, the request method is not allowed to be changed when reissuing the original request. For example, a POST request should be repeated using another POST request.
        Temporary_Redirect = 307,
        ///The request and all future requests should be repeated using another URI. 307 and 308 parallel the behaviors of 302 and 301, but do not allow the HTTP method to change. So, for example, submitting a form to a permanently redirected resource may continue smoothly.
        Permanent_Redirect = 308,
        ///The server cannot or will not process the request due to an apparent client error.
        Bad_Request = 400,
        ///Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided.
        Unauthorized = 401,
        ///Reserved for future use. The original intention was that this code might be used as part of some form of digital cash or micropayment scheme, as proposed, for example, by GNU Taler, but that has not yet happened, and this code is not widely used.
        Payment_Required = 402,
        ///The request contained valid data and was understood by the server, but the server is refusing action. This may be due to the user not having the necessary permissions for a resource or needing an account of some sort, or attempting a prohibited action.
        Forbidden = 403,
        ///The requested resource could not be found but may be available in the future. Subsequent requests by the client are permissible.
        Not_Found = 404,
        ///A request method is not supported for the requested resource; for example, a GET request on a form that requires data to be presented via POST, or a PUT request on a read-only resource.
        Method_Not_Allowed = 405,
        ///The requested resource is capable of generating only content not acceptable according to the Accept headers sent in the request.
        Not_Acceptable = 406,
        ///The client must first authenticate itself with the proxy.
        Proxy_Authentication_Required = 407,
        ///The server timed out waiting for the request.
        Request_Timeout = 408,
        ///Indicates that the request could not be processed because of conflict in the current state of the resource, such as an edit conflict between multiple simultaneous updates.
        Conflict = 409,
        ///Indicates that the resource requested is no longer available and will not be available again.
        Gone = 410,
        ///The request did not specify the length of its content, which is required by the requested resource.
        Length_Required = 411,
        ///The server does not meet one of the preconditions that the requester put on the request header fields.
        Precondition_Failed = 412,
        ///The request is larger than the server is willing or able to process.
        Request_Entity_Too_Large = 413,
        ///The URI provided was too long for the server to process.
        Request_URI_Too_Long = 414,
        ///The request entity has a media type which the server or resource does not support.
        Unsupported_Media_Type = 415,
        ///The client has asked for a portion of the file (byte serving), but the server cannot supply that portion.
        Requested_Range_Not_Satisfiable = 416,
        ///The server cannot meet the requirements of the Expect request-header field.
        Expectation_Failed = 417,
        ///The request was directed at a server that is not able to produce a respons
        Misdirected_Request = 421,
        ///The request was well-formed but was unable to be followed due to semantic errors.
        Unprocessable_Entity = 422,
        ///The resource that is being accessed is locked.
        Locked = 423,
        ///The request failed because it depended on another request and that request failed.
        Failed_Dependency = 424,
        ///Indicates that the server is unwilling to risk processing a request that might be replayed.
        Too_Early = 425,
        ///The client should switch to a different protocol such as TLS/1.3, given in the Upgrade header field.
        Upgrade_Required = 426,
        ///The origin server requires the request to be conditional. Intended to prevent the 'lost update' problem, where a client GETs a resource's state, modifies it, and PUTs it back to the server, when meanwhile a third party has modified the state on the server, leading to a conflict.
        Precondition_Required = 428,
        ///The user has sent too many requests in a given amount of time. Intended for use with rate-limiting schemes.
        Too_Many_Requests = 429,
        ///The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large.
        Request_Header_Fields_Too_Large = 431,
        ///A server operator has received a legal demand to deny access to a resource or to a set of resources that includes the requested resource.
        Unavailable_For_Legal_Reasons = 451,
        ///A generic error message, given when an unexpected condition was encountered and no more specific message is suitable.
        Internal_Server_Error = 500,
        ///The server either does not recognize the request method, or it lacks the ability to fulfil the request. Usually this implies future availability.
        Not_Implemented = 501,
        ///The server was acting as a gateway or proxy and received an invalid response from the upstream server.
        Bad_Gateway = 502,
        ///The server cannot handle the request (because it is overloaded or down for maintenance). Generally, this is a temporary state.
        Service_Unavailable = 503,
        ///The server was acting as a gateway or proxy and did not receive a timely response from the upstream server.
        Gateway_Timeout = 504,
        ///The server does not support the HTTP protocol version used in the request.
        HTTP_Version_Not_Supported = 505,
        ///Transparent content negotiation for the request results in a circular reference.
        Variant_Also_Negotiates = 506,
        ///The server is unable to store the representation needed to complete the request.
        Insufficient_Storage = 507,
        ///The server detected an infinite loop while processing the request.
        Loop_Detected = 508,
        ///Further extensions to the request are required for the server to fulfil it.
        Not_Extended = 510,
        ///The client needs to authenticate to gain network access. Intended for use by intercepting proxies used to control access to the network.
        Network_Authentication_Required = 511
    };

    auto getHttpStatusDetails(HttpStatus status) -> const std::string&;
} // namespace tristan::network
#endif // HTTP_STATUS_CODES_HPP