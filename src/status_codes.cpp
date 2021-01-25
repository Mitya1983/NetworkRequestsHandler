#include "status_codes.hpp"

#include <vector>

namespace {

inline const std::vector<std::pair<std::string, std::string>> statusCodesDescription =
{
    {"100", "The server has received the request headers and the client should proceed to send the request body"},
    {"101", "The requester has asked the server to switch protocols and the server has agreed to do so."},
    {"102", "This code indicates that the server has received and is processing the request, but no response is available yet."},
    {"103", "Used to return some response headers before final HTTP message."},
    {"200", "Standard response for successful HTTP requests"},
    {"201", "The request has been fulfilled, resulting in the creation of a new resource."},
    {"202", "The request has been accepted for processing, but the processing has not been completed. The request might or might not be eventually acted upon, and may be disallowed when processing occurs."},
    {"203", "The server is a transforming proxy (e.g. a Web accelerator) that received a 200 OK from its origin, but is returning a modified version of the origin's response."},
    {"204", "The server successfully processed the request, and is not returning any content."},
    {"205", "The server successfully processed the request, asks that the requester reset its document view, and is not returning any content."},
    {"206", "The server is delivering only part of the resource (byte serving) due to a range header sent by the client. The range header is used by HTTP clients to enable resuming of interrupted downloads, or split a download into multiple simultaneous streams."},
    {"207","The message body that follows is by default an XML message and can contain a number of separate response codes, depending on how many sub-requests were made."},
    {"208","The members of a DAV binding have already been enumerated in a preceding part of the (multistatus) response, and are not being included again."},
    {"226","The server has fulfilled a request for the resource, and the response is a representation of the result of one or more instance-manipulations applied to the current instance."},
    {"300", "Indicates multiple options for the resource from which the client may choose."},
    {"301", "This and all future requests should be directed to the given URI"},
    {"302", "Tells the client to look at (browse to) another URL. 302 has been superseded by 303 and 307."},
    {"303", "The response to the request can be found under another URI using the GET method. When received in response to a POST (or PUT/DELETE), the client should presume that the server has received the data and should issue a new GET request to the given URI."},
    {"304", "Indicates that the resource has not been modified since the version specified by the request headers"},
    {"305", "The requested resource is available only through a proxy, the address for which is provided in the response."},
    {"306", "No longer used. Originally meant \"Subsequent requests should use the specified proxy.\""},
    {"307", "In this case, the request should be repeated with another URI; however, future requests should still use the original URI. In contrast to how 302 was historically implemented, the request method is not allowed to be changed when reissuing the original request. For example, a POST request should be repeated using another POST request."},
    {"308", "The request and all future requests should be repeated using another URI. 307 and 308 parallel the behaviors of 302 and 301, but do not allow the HTTP method to change. So, for example, submitting a form to a permanently redirected resource may continue smoothly."},
    {"400", "The server cannot or will not process the request due to an apparent client error"},
    {"401", "Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided."},
    {"402", "Reserved for future use. The original intention was that this code might be used as part of some form of digital cash or micropayment scheme, as proposed, for example, by GNU Taler, but that has not yet happened, and this code is not widely used."},
    {"403", "The request contained valid data and was understood by the server, but the server is refusing action. This may be due to the user not having the necessary permissions for a resource or needing an account of some sort, or attempting a prohibited action."},
    {"404", "The requested resource could not be found but may be available in the future. Subsequent requests by the client are permissible."},
    {"405", "A request method is not supported for the requested resource; for example, a GET request on a form that requires data to be presented via POST, or a PUT request on a read-only resource."},
    {"406", "The requested resource is capable of generating only content not acceptable according to the Accept headers sent in the request."},
    {"407", "The client must first authenticate itself with the proxy."},
    {"408", "The server timed out waiting for the request."},
    {"409", "Indicates that the request could not be processed because of conflict in the current state of the resource, such as an edit conflict between multiple simultaneous updates."},
    {"410", "Indicates that the resource requested is no longer available and will not be available again."},
    {"411", "The request did not specify the length of its content, which is required by the requested resource."},
    {"412", "The server does not meet one of the preconditions that the requester put on the request header fields."},
    {"413", "The request is larger than the server is willing or able to process."},
    {"414", "The URI provided was too long for the server to process."},
    {"415", "The request entity has a media type which the server or resource does not support."},
    {"416", "The client has asked for a portion of the file (byte serving), but the server cannot supply that portion."},
    {"417", "The server cannot meet the requirements of the Expect request-header field."},
    {"421", "The request was directed at a server that is not able to produce a respons"},
    {"422", "The request was well-formed but was unable to be followed due to semantic errors."},
    {"423", "The resource that is being accessed is locked."},
    {"424", "The request failed because it depended on another request and that request failed."},
    {"425", "Indicates that the server is unwilling to risk processing a request that might be replayed."},
    {"426", "The client should switch to a different protocol such as TLS/1.3, given in the Upgrade header field."},
    {"428", "The origin server requires the request to be conditional. Intended to prevent the 'lost update' problem, where a client GETs a resource's state, modifies it, and PUTs it back to the server, when meanwhile a third party has modified the state on the server, leading to a conflict."},
    {"429", "The user has sent too many requests in a given amount of time. Intended for use with rate-limiting schemes."},
    {"431", "The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large."},
    {"451", "A server operator has received a legal demand to deny access to a resource or to a set of resources that includes the requested resource."},
    {"500", "A generic error message, given when an unexpected condition was encountered and no more specific message is suitable."},
    {"501", "Not Implemented"},
    {"502", "Bad Gateway"},
    {"503", "Service Unavailable"},
    {"504", "Gateway Timeout"},
    {"505", "The server does not support the HTTP protocol version used in the request."},
    {"506", "Transparent content negotiation for the request results in a circular reference."},
    {"507", "The server is unable to store the representation needed to complete the request."},
    {"508", "The server detected an infinite loop while processing the request."},
    {"510", "Further extensions to the request are required for the server to fulfil it."},
    {"511", "The client needs to authenticate to gain network access. Intended for use by intercepting proxies used to control access to the network."},
};

} // namespace
std::pair<std::string, std::string> network::getHttpStatusDetails(network::HttpStatus status)
{
    return statusCodesDescription.at(static_cast<size_t>(status));
}
