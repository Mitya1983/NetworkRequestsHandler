#ifndef HTTP_HEADER_NAMES_HPP
#define HTTP_HEADER_NAMES_HPP

#include <string>

namespace tristan::network::http::header_names {
    
    [[maybe_unused]] inline const std::string a_im = "A-IM";
    [[maybe_unused]] inline const std::string accept = "accept";
    [[maybe_unused]] inline const std::string accept_charset = "accept-charset";
    [[maybe_unused]] inline const std::string accept_datetime = "accept-datetime";
    [[maybe_unused]] inline const std::string accept_encoding = "accept-encoding";
    [[maybe_unused]] inline const std::string accept_language = "accept-language";
    [[maybe_unused]] inline const std::string access_control_allow_credentials = "access-control-allow-credentials";
    [[maybe_unused]] inline const std::string access_control_allow_headers = "access-control-allow-headers";
    [[maybe_unused]] inline const std::string access_control_allow_methods = "access-control-allow-methods";
    [[maybe_unused]] inline const std::string access_control_allow_origin = "access-control-allow-origin";
    [[maybe_unused]] inline const std::string access_control_expose_headers = "access-control-expose-headers";
    [[maybe_unused]] inline const std::string access_control_max_age = "access-control-max-age";
    [[maybe_unused]] inline const std::string access_control_request_method = "access-control-request-method";
    [[maybe_unused]] inline const std::string access_control_request_headers = "access-control-request-headers";
    [[maybe_unused]] inline const std::string accept_patch = "accept-patch";
    [[maybe_unused]] inline const std::string accept_ranges = "accept-ranges";
    [[maybe_unused]] inline const std::string age = "age";
    [[maybe_unused]] inline const std::string allow = "allow";
    [[maybe_unused]] inline const std::string alt_svc = "alt-svc";
    [[maybe_unused]] inline const std::string alternate_protocol = "alternate-protocol";
    [[maybe_unused]] inline const std::string authorization = "authorization";
    [[maybe_unused]] inline const std::string cache_control = "cache-control";
    [[maybe_unused]] inline const std::string client_date = "client-date";
    [[maybe_unused]] inline const std::string client_peer = "client-peer";
    [[maybe_unused]] inline const std::string client_response_num = "client-response-num";
    [[maybe_unused]] inline const std::string connection = "connection";
    [[maybe_unused]] inline const std::string content_disposition = "content-disposition";
    [[maybe_unused]] inline const std::string content_encoding = "content-encoding";
    [[maybe_unused]] inline const std::string content_language = "content-language";
    [[maybe_unused]] inline const std::string content_length = "content-length";
    [[maybe_unused]] inline const std::string content_location = "content-location";
    [[maybe_unused]] inline const std::string content_md_5 = "content-md5";
    [[maybe_unused]] inline const std::string content_range = "content-range";
    [[maybe_unused]] inline const std::string content_security_policy = "content-security-policy";
    [[maybe_unused]] inline const std::string content_security_policy_report_only = "content-security-policy-report-only";
    [[maybe_unused]] inline const std::string content_type = "content-type";
    [[maybe_unused]] inline const std::string date = "date";
    [[maybe_unused]] inline const std::string delta_base = "delta-base";
    [[maybe_unused]] inline const std::string dnt = "DNT";
    [[maybe_unused]] inline const std::string etag = "etag";
    [[maybe_unused]] inline const std::string expect = "expect";
    [[maybe_unused]] inline const std::string expires = "expires";
    [[maybe_unused]] inline const std::string forwarded = "forwarded";
    [[maybe_unused]] inline const std::string from = "from";
    [[maybe_unused]] inline const std::string front_end_https = "front-end-https";
    [[maybe_unused]] inline const std::string host = "host";
    [[maybe_unused]] inline const std::string http_2_settings = "HTTP2-settings";
    [[maybe_unused]] inline const std::string im = "IM";
    [[maybe_unused]] inline const std::string if_match = "if-match";
    [[maybe_unused]] inline const std::string if_modified_since = "if-modified-since";
    [[maybe_unused]] inline const std::string if_none_match = "if-none-match";
    [[maybe_unused]] inline const std::string if_range = "if-range";
    [[maybe_unused]] inline const std::string if_unmodified_since = "if-unmodified-since";
    [[maybe_unused]] inline const std::string keep_alive = "keep-alive";
    [[maybe_unused]] inline const std::string last_modified = "last-modified";
    [[maybe_unused]] inline const std::string link = "link";
    [[maybe_unused]] inline const std::string location = "location";
    [[maybe_unused]] inline const std::string max_forwards = "max-forwards";
    [[maybe_unused]] inline const std::string origin = "origin";
    [[maybe_unused]] inline const std::string p_3_p = "p3p";
    [[maybe_unused]] inline const std::string pragma = "pragma";
    [[maybe_unused]] inline const std::string prefer = "prefer";
    [[maybe_unused]] inline const std::string preference_applied = "preference-applied";
    [[maybe_unused]] inline const std::string proxy_authenticate = "proxy-authenticate";
    [[maybe_unused]] inline const std::string proxy_authorization = "proxy-authorization";
    [[maybe_unused]] inline const std::string proxy_connection = "proxy-connection";
    [[maybe_unused]] inline const std::string public_key_pins = "public-key-pins";
    [[maybe_unused]] inline const std::string range = "range";
    [[maybe_unused]] inline const std::string referer = "referer";
    [[maybe_unused]] inline const std::string refresh = "refresh";
    [[maybe_unused]] inline const std::string retry_after = "retry-after";
    [[maybe_unused]] inline const std::string server = "server";
    [[maybe_unused]] inline const std::string set_cookie = "set-cookie";
    [[maybe_unused]] inline const std::string status = "status";
    [[maybe_unused]] inline const std::string strict_transport_security = "strict-transport-security";
    [[maybe_unused]] inline const std::string timing_allow_origin = "timing-allow-origin";
    [[maybe_unused]] inline const std::string tk = "tk";
    [[maybe_unused]] inline const std::string trailer = "trailer";
    [[maybe_unused]] inline const std::string transfer_encoding = "transfer-encoding";
    [[maybe_unused]] inline const std::string upgrade = "upgrade";
    [[maybe_unused]] inline const std::string upgrade_insecure_requests = "upgrade-insecure-requests";
    [[maybe_unused]] inline const std::string user_agent = "user-agent";
    [[maybe_unused]] inline const std::string vary = "vary";
    [[maybe_unused]] inline const std::string via = "via";
    [[maybe_unused]] inline const std::string warning = "warning";
    [[maybe_unused]] inline const std::string www_authenticate = "www-authenticate";
    [[maybe_unused]] inline const std::string x_aspnet_version = "x-aspnet-version";
    [[maybe_unused]] inline const std::string x_content_type_options = "x-content-type-options";
    [[maybe_unused]] inline const std::string x_content_security_policy = "x-content-security-policy";
    [[maybe_unused]] inline const std::string x_forwarded_for = "x-forwarded-for";
    [[maybe_unused]] inline const std::string x_forwarded_host = "x-forwarded-host";
    [[maybe_unused]] inline const std::string x_forwarded_proto = "x-forwarded-proto";
    [[maybe_unused]] inline const std::string x_frame_options = "x-frame-options";
    [[maybe_unused]] inline const std::string x_http_method_override = "x-http-method-override";
    [[maybe_unused]] inline const std::string x_permitted_cross_domain_policies = "x-permitted-cross-domain-policies";
    [[maybe_unused]] inline const std::string x_pingback = "x-pingback";
    [[maybe_unused]] inline const std::string x_powered_by = "x-powered-by";
    [[maybe_unused]] inline const std::string x_requested_with = "x-requested-with";
    [[maybe_unused]] inline const std::string x_robots_tag = "x-robots-tag";
    [[maybe_unused]] inline const std::string x_ua_compatible = "x-ua-compatible";
    [[maybe_unused]] inline const std::string x_webrit_csp = "x-webrit-csp";
    [[maybe_unused]] inline const std::string x_xss_protection = "x-xss-protection";


} // namespace tristan::network::http::header_names

#endif // HTTP_HEADER_NAMES_HPP
