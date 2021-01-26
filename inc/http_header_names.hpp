#ifndef HTTP_HEADER_NAMES_HPP
#define HTTP_HEADER_NAMES_HPP

#include <string>

namespace network::http::headernames {

inline const std::string access_control_allow_credentials = "access-control-allow-credentials";
inline const std::string access_control_allow_headers = "access-control-allow-headers";
inline const std::string access_control_allow_methods = "access-control-allow-methods";
inline const std::string access_control_allow_origin = "access-control-allow-origin";
inline const std::string access_control_expose_headers = "access-control-expose-headers";
inline const std::string access_control_max_age = "access-control-max-age";
inline const std::string accept_ranges = "accept-ranges";
inline const std::string age = "age";
inline const std::string allow = "allow";
inline const std::string alternate_protocol = "alternate-protocol";
inline const std::string cache_control = "cache-control";
inline const std::string client_date = "client-date";
inline const std::string client_peer = "client-peer";
inline const std::string client_response_num = "client-response-num";
inline const std::string connection = "connection";
inline const std::string content_disposition = "content-disposition";
inline const std::string content_encoding = "content-encoding";
inline const std::string content_language = "content-language";
inline const std::string content_length = "content-length";
inline const std::string content_location = "content-location";
inline const std::string content_md5 = "content-md5";
inline const std::string content_range = "content-range";
inline const std::string content_security_policy = "content-security-policy";
inline const std::string content_security_policy_report_only = "content-security-policy-report-only";
inline const std::string content_type = "content-type";
inline const std::string date = "date";
inline const std::string etag = "etag";
inline const std::string expires = "expires";
inline const std::string keep_alive = "keep-alive";
inline const std::string last_modified = "last-modified";
inline const std::string link = "link";
inline const std::string location = "location";
inline const std::string p3p = "p3p";
inline const std::string pragma = "pragma";
inline const std::string proxy_authenticate = "proxy-authenticate";
inline const std::string proxy_connection = "proxy-connection";
inline const std::string refresh = "refresh";
inline const std::string retry_after = "retry-after";
inline const std::string server = "server";
inline const std::string set_cookie = "set-cookie";
inline const std::string status = "status";
inline const std::string strict_transport_security = "strict-transport-security";
inline const std::string timing_allow_origin = "timing-allow-origin";
inline const std::string trailer = "trailer";
inline const std::string transfer_encoding = "transfer-encoding";
inline const std::string upgrade = "upgrade";
inline const std::string vary = "vary";
inline const std::string via = "via";
inline const std::string warning = "warning";
inline const std::string www_authenticate = "www-authenticate";
inline const std::string x_aspnet_version = "x-aspnet-version";
inline const std::string x_content_type_options = "x-content-type-options";
inline const std::string x_content_security_policy = "x-content-security-policy";
inline const std::string x_frame_options = "x-frame-options";
inline const std::string x_permitted_cross_domain_policies = "x-permitted-cross-domain-policies";
inline const std::string x_pingback = "x-pingback";
inline const std::string x_powered_by = "x-powered-by";
inline const std::string x_robots_tag = "x-robots-tag";
inline const std::string x_ua_compatible = "x-ua-compatible";
inline const std::string x_webrit_csp = "x-webrit-csp";
inline const std::string x_xss_protection = "x-xss-protection";


} // namespace network::http::headernames

#endif // HTTP_HEADER_NAMES_HPP
