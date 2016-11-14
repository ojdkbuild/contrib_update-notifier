/* 
 * File:   Config.hpp
 * Author: alex
 *
 * Created on October 20, 2016, 11:13 PM
 */

#ifndef UPDATE_CHECKER_CONFIG_HPP
#define	UPDATE_CHECKER_CONFIG_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "JsonRecord.hpp"

namespace checker {

class Config {
public:    
    uint32_t shipped_version_number;
    std::string remote_version_url;
    uint32_t max_json_size_bytes;
    std::string version_filename;
    uint32_t max_path_length;
    std::string vendor_name;
    std::string application_name;
    std::string work_directory;
    
    // curl general behavior options
    uint32_t curl_max_connects;
    uint32_t curl_max_bufsize_bytes;
    uint32_t curl_fdset_timeout_millis;
    bool curl_force_http_10;
    bool curl_noprogress;
    bool curl_nosignal;
    
    // curl TCP options
    bool curl_tcp_nodelay;
    uint32_t curl_connecttimeout_millis;
    
    // curl HTTP options
    uint32_t curl_buffersize_bytes;
    std::string curl_accept_encoding;
    bool curl_followlocation;
    uint32_t curl_maxredirs;
    std::string curl_useragent;
    
    // curl throttling options
    uint32_t curl_max_sent_speed_large_bytes_per_second;
    uint32_t curl_max_recv_speed_large_bytes_per_second;
    
    // curl SSL options
    std::string curl_sslcert_filename;
    std::string curl_sslcertype;
    std::string curl_sslkey_filename;
    std::string curl_ssl_key_type;
    std::string curl_ssl_keypasswd;
    bool curl_require_tls;
    bool curl_ssl_verifyhost;
    bool curl_ssl_verifypeer;
    std::string curl_cainfo_filename;
    std::string curl_crlfile_filename;
    std::string curl_ssl_cipher_list;
    
    std::vector<std::pair<std::string, std::string> > curl_headers;
    
    Config() :
    shipped_version_number(0),
    max_json_size_bytes(0),
    max_path_length(0),
    
    curl_max_connects(0),
    curl_max_bufsize_bytes(0),
    curl_fdset_timeout_millis(0),
    curl_force_http_10(false),
    curl_noprogress(false),
    curl_nosignal(false),
    
    curl_tcp_nodelay(false),
    curl_connecttimeout_millis(0),
    
    curl_buffersize_bytes(0),
    curl_followlocation(false),
    curl_maxredirs(0),
    
    curl_max_sent_speed_large_bytes_per_second(0),
    curl_max_recv_speed_large_bytes_per_second(0),
    
    curl_require_tls(false),
    curl_ssl_verifyhost(false),
    curl_ssl_verifypeer(false)
    { }
    
    Config(const JsonRecord& json) :
    shipped_version_number(json.get_uint32("shipped_version_number", -1)),
    remote_version_url(json.get_string("remote_version_url")),
    max_json_size_bytes(json.get_uint32("max_json_size_bytes", 1 << 15)),
    version_filename(json.get_string("version_filename")),
    max_path_length(json.get_uint32("max_path_length", 1 << 10)),
    vendor_name(json.get_string("vendor_name", "ojdkbuild")),
    application_name(json.get_string("application_name", "update_checker")),
    work_directory(json.get_string("work_directory", ".")),
    
    curl_max_connects(json.get_uint32("curl_max_connects", 1)), 
    curl_max_bufsize_bytes(json.get_uint32("curl_max_bufsize_bytes", 1 << 15)),
    curl_fdset_timeout_millis(json.get_uint32("curl_fdset_timeout_millis", 100)),
    curl_force_http_10(json.get_bool("curl_force_http_10", true)),
    curl_noprogress(json.get_bool("curl_noprogress", true)),
    curl_nosignal(json.get_bool("curl_nosignal", true)),
    
    curl_tcp_nodelay(json.get_bool("curl_tcp_nodelay", false)),
    curl_connecttimeout_millis(json.get_uint32("curl_connecttimeout_millis", 10000)),
    
    curl_buffersize_bytes(json.get_uint32("curl_buffersize_bytes", 16384)),
    curl_accept_encoding(json.get_string("curl_accept_encoding", "gzip")),
    curl_followlocation(json.get_bool("curl_followlocation", true)),
    curl_maxredirs(json.get_uint32("curl_maxredirs", 1 << 5)),
    curl_useragent(json.get_string("curl_useragent")),
    
    curl_max_sent_speed_large_bytes_per_second(json.get_uint32("curl_max_sent_speed_large_bytes_per_second")),
    curl_max_recv_speed_large_bytes_per_second(json.get_uint32("curl_max_recv_speed_large_bytes_per_second")),
    
    curl_sslcert_filename(json.get_string("curl_sslcert_filename")),
    curl_sslcertype(json.get_string("curl_sslcertype")),
    curl_sslkey_filename(json.get_string("curl_sslkey_filename")),
    curl_ssl_key_type(json.get_string("curl_ssl_key_type")),
    curl_ssl_keypasswd(json.get_string("curl_ssl_keypasswd")),
    curl_require_tls(json.get_bool("curl_require_tls", true)),
    curl_ssl_verifyhost(json.get_bool("curl_ssl_verifyhost", true)),
    curl_ssl_verifypeer(json.get_bool("curl_ssl_verifypeer", true)),
    curl_cainfo_filename(json.get_string("curl_cainfo_filename")),
    curl_crlfile_filename(json.get_string("curl_crlfile_filename")),
    curl_ssl_cipher_list(json.get_string("curl_ssl_cipher_list"))
    
    { 
        // todo: parse headers
    }
};

} // namespace

#endif	/* UPDATE_CHECKER_CONFIG_HPP */

