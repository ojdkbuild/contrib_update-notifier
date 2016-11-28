/*
 * Copyright 2016, akashche at redhat.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   fetchurl.cpp
 * Author: alex
 * 
 * Created on October 20, 2016, 11:00 PM
 */

#include "fetchurl.hpp"

#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "curl/curl.h"
#include "jansson.h"

#include "CheckerException.hpp"
#include "utils.hpp"
#include "platform.hpp"


namespace checker {

namespace { // anonymous


class CurlGlobalInitializer {
public:
    CurlGlobalInitializer() {
        CURLcode err_init = curl_global_init(CURL_GLOBAL_ALL);
        if (CURLE_OK != err_init) {
            throw CheckerException("'curl_global_init' error: [" + std::string(curl_easy_strerror(err_init)) + "]");
        }
    }
    
    ~CurlGlobalInitializer() {
        curl_global_cleanup();
    }
};


class CurlMultiHolder {
    CURLM* curlm;
    
public:
    CurlMultiHolder(CURLM* curlm) :
    curlm(curlm) { 
        if (!this->curlm) {
            throw CheckerException("Invalid 'null' CURLM specified");
        }
    }

    ~CurlMultiHolder() {
        curl_multi_cleanup(curlm);
    }

    CURLM* get() {
        return curlm;
    }
    
private:
    CurlMultiHolder(const CurlMultiHolder& other);
    
    CurlMultiHolder& operator=(const CurlMultiHolder& other);
    
};


class CurlEasyHolder {
    CurlMultiHolder& multi;
    CURL* curl;
    
public:
    
    CurlEasyHolder(CurlMultiHolder& multi, CURL* curl) :
    multi(multi),
    curl(curl) { 
        if (!this->multi.get()) {
            throw CheckerException("Invalid 'null' CURLM specified");
        }
        if (!this->curl) {
            throw CheckerException("Invalid 'null' CURL specified");
        }
        CURLMcode errm = curl_multi_add_handle(this->multi.get(), this->curl);
        if (errm != CURLM_OK) {
            throw CheckerException("cURL multi_add error: [" + utils::to_string(errm) + "]");
        }
    }

    ~CurlEasyHolder() {
        curl_multi_remove_handle(multi.get(), curl);
        curl_easy_cleanup(curl);
    }
    
    CURL* get() {
        return curl;
    }
    
private:
    CurlEasyHolder(const CurlEasyHolder& other);

    CurlEasyHolder& operator=(const CurlEasyHolder& other);
    
};

class AppliedHeaders {
    std::vector<std::string> headers;
    struct curl_slist* slist;

public:
    AppliedHeaders() :
    slist(NULL) { }

    ~AppliedHeaders() {
        if (slist) {
            curl_slist_free_all(slist);
        }
    }
    
    void add_header(const std::string& header) {
        headers.push_back(header);
    }

    const std::string& get_last_header() {
        return headers.back();
    }

    void set_slist(struct curl_slist* slist) {
        this->slist = slist;
    }

private:
    AppliedHeaders(const AppliedHeaders& other);

    AppliedHeaders& operator=(const AppliedHeaders& other);    
};


class FetchCtx {
public:        
    const Config& cf;
    CurlMultiHolder& multi;
    CurlEasyHolder& curl;

    AppliedHeaders applied_headers;
    std::vector<char> buf;
    size_t buf_idx;
    bool open;
    bool headers_received;
    long response_code;
    
    std::string error;
        
    FetchCtx(const Config& cf, CurlMultiHolder& multi, CurlEasyHolder& curl):
    cf(cf),
    multi(multi),
    curl(curl),
            
    buf_idx(0),
    open(true),
    headers_received(false),
    response_code(-1) {}
    
    void append_error(const std::string& err_msg) {
        if (err_msg.empty()) return;
        if (!this->error.empty()) {
            this->error.append("\n");
        }
        this->error.append(err_msg);
    }
    
private:
    FetchCtx(const FetchCtx& other);

    FetchCtx& operator=(const FetchCtx& other);
};

// curl utils

void setmopt_uint32(CurlMultiHolder& multi, CURLMoption opt, uint32_t value) {
    CURLMcode err = curl_multi_setopt(multi.get(), opt, value);
    if (err != CURLM_OK) {
        throw CheckerException(
            "Error setting multi option: [" + utils::to_string(opt) + "]," +
            " to value: [" + utils::to_string(value) + "]");
    }
}

long getinfo_long(CurlEasyHolder& curl, CURLINFO opt) {
    long out = -1;
    CURLcode err = curl_easy_getinfo(curl.get(), opt, &out);
    if (err != CURLE_OK) {
        throw CheckerException(
            "cURL curl_easy_getinfo error: [" + utils::to_string(err) + "]," +
            " option: [" + utils::to_string(opt) + "]");
    }
    return out;
}

// note: think about integer overflow 
void setopt_uint32(CurlEasyHolder& curl, CURLoption opt, uint32_t value) {
    if (0 == value) {
        return;
    }
    if (value > static_cast<uint32_t> (std::numeric_limits<int32_t>::max())) {
        throw CheckerException(
            "Error setting option: [" + utils::to_string(opt) + "]," +
            " to invalid overflow value: [" + utils::to_string(value) + "]");
    }
    CURLcode err = curl_easy_setopt(curl.get(), opt, static_cast<long> (value));
    if (err != CURLE_OK) {
        throw CheckerException(
            "Error setting option: [" + utils::to_string(opt) + "]," +
            " to value: [" + utils::to_string(value) + "]");
    }
}

void setopt_bool(CurlEasyHolder& curl, CURLoption opt, bool value) {
    CURLcode err = curl_easy_setopt(curl.get(), opt, value ? 1 : 0);
    if (err != CURLE_OK) {
        throw CheckerException("Error setting option: [" + utils::to_string(opt) + "]," +
            " to value: [" + utils::to_string(value) + "]");
    }
}

void setopt_string(CurlEasyHolder& curl, CURLoption opt, const std::string& value) {
    if (value.empty()) {
        return;
    }
    CURLcode err = curl_easy_setopt(curl.get(), opt, value.c_str());
    if (err != CURLE_OK) {
        throw CheckerException(
            "Error setting option: [" + utils::to_string(opt) + "]," +
            " to value: [" + value + "]");
    }
}

void setopt_object(CurlEasyHolder& curl, CURLoption opt, void* value) {
    if (!value) {
        return;
    }
    CURLcode err = curl_easy_setopt(curl.get(), opt, value);
    if (err != CURLE_OK) {
        throw CheckerException(
            "Error setting option: [" + utils::to_string(opt) + "]");
    }
}

struct curl_slist* append_header(struct curl_slist* slist, const std::string& header) {
    struct curl_slist* res = curl_slist_append(slist, header.c_str());
    if (!res) {
        throw CheckerException("Error appending header: [" + header + "]");
    }
    return res;
}

void apply_headers(FetchCtx& ctx) {
    if (ctx.cf.curl_headers.empty()) return;
    struct curl_slist* slist = NULL;    
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = ctx.cf.curl_headers.begin();
            it != ctx.cf.curl_headers.end(); ++it) {
        const std::pair<std::string, std::string>& en = *it;
        ctx.applied_headers.add_header(en.first + ": " + en.second);
        slist = append_header(slist, ctx.applied_headers.get_last_header());
        ctx.applied_headers.set_slist(slist);
    }
    setopt_object(ctx.curl, CURLOPT_HTTPHEADER, static_cast<void*> (slist));
}

// receive response

size_t write(FetchCtx& ctx, char *buffer, size_t size, size_t nitems) {
    size_t len = size*nitems;
    if (len > ctx.cf.curl_max_bufsize_bytes) {
        throw CheckerException("'curl_max_bufsize_bytes' exceeded: [" + utils::to_string(len) + "]");
    }
    ctx.buf.resize(len);
    memcpy(ctx.buf.data(), buffer, len);
    ctx.cf.trace("data chunk received, size: [" + utils::to_string(ctx.buf.size()) + "]");
    ctx.cf.trace("EVENT_RESPONSE " + std::string(ctx.buf.data(), ctx.buf.size()));
    return len;
}

void check_state_after_perform(FetchCtx& ctx) {
    ctx.cf.trace("connection closed, headers: [" + utils::to_string(ctx.headers_received) + "]");
    ctx.cf.trace("connection closed, response_code: [" + utils::to_string(ctx.response_code) + "]");
    
    // check whether connection was successful
    if (!ctx.headers_received) {
        throw CheckerException("HTTP connection error");
    }

    // check for response code
    if (ctx.response_code >= 400) {
        throw CheckerException(std::string() + "HTTP error returned from server" +
                " response_code: [" + utils::to_string(ctx.response_code) + "]");
    }

    // note: maybe check for timeout
}

// http://curl-library.cool.haxx.narkive.com/2sYifbgu/issue-with-curl-multi-timeout-while-doing-non-blocking-http-posts-in-vms
struct timeval create_timeout_struct(long timeo) {
    struct timeval timeout;
    std::memset(&timeout, '\0', sizeof (timeout));
    timeout.tv_sec = 10;
    if (timeo > 0) {
        long ctsecs = timeo / 1000;
        if (ctsecs < 10) {
            timeout.tv_sec = ctsecs;
        }
        timeout.tv_usec = (timeo % 1000) * 1000;
    }
    return timeout;
}

fd_set create_fd() {
    fd_set res;
    FD_ZERO(&res);
    return res;
}

// curl_multi_socket_action may be used instead of fdset with newer versions of curl
void fill_buffer(FetchCtx& ctx) {
    ctx.buf_idx = 0;
    ctx.buf.resize(0);
    ctx.cf.trace("fill buffer attempt");
    // attempt to fill buffer
    while (ctx.open && 0 == ctx.buf.size()) {
        long timeo = -1;
        CURLMcode err_timeout = curl_multi_timeout(ctx.multi.get(), &timeo);
        if (err_timeout != CURLM_OK) {
            throw CheckerException(
                "cURL timeout error: [" + utils::to_string(err_timeout) + "]");
        }
        struct timeval timeout = create_timeout_struct(timeo);

        // get file descriptors from the transfers
        fd_set fdread = create_fd();
        fd_set fdwrite = create_fd();
        fd_set fdexcep = create_fd();
        int maxfd = -1;
        CURLMcode err_fdset = curl_multi_fdset(ctx.multi.get(), &fdread, &fdwrite, &fdexcep, &maxfd);
        if (err_fdset != CURLM_OK) {
            throw CheckerException(
                "cURL fdset error: [" + utils::to_string(err_fdset) + "]");
        }

        // wait or select
        int err_select = 0;
        if (maxfd == -1) {
            ctx.cf.trace("fdset fail, enter sleep branch");
            platform::thread_sleep_millis(ctx.cf.curl_fdset_timeout_millis);
        } else {
            ctx.cf.trace("fdset success, performing select");
            err_select = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
        }

        // do perform if no select error
        if (-1 != err_select) {
            int active = -1;
            CURLMcode err = curl_multi_perform(ctx.multi.get(), &active);
            if (err != CURLM_OK) {
                throw CheckerException(
                    "cURL multi_perform error: [" + utils::to_string(err) + "]");
            }
            ctx.cf.trace("perform complete, result: [" + utils::to_string(active) + "]");
            ctx.open = (1 == active);
            if (!ctx.open) {
                check_state_after_perform(ctx);
            }
        } else {
            ctx.cf.trace("select fail");
        }
    }
}

size_t write_headers(FetchCtx& ctx, char* /* buffer */, size_t size, size_t nitems) {
    ctx.headers_received = true;
    ctx.response_code = getinfo_long(ctx.curl, CURLINFO_RESPONSE_CODE);
    return size * nitems;
}

size_t read(FetchCtx& ctx, void* buffer, size_t size) {
    ctx.cf.trace("response chunk read, size: [" + utils::to_string(size) + "]");
    if (ctx.buf_idx == ctx.buf.size()) {
        ctx.cf.trace("buffer empty");
        if (!ctx.open) {
            ctx.cf.trace("connection closed");
            return 0;
        }
        fill_buffer(ctx);
        if (0 == ctx.buf.size()) {
            ctx.cf.trace("zero read");
            // currently zero curl read is not handled
            return 0;
        }
    }
    // return from buffer
    size_t avail = ctx.buf.size() - ctx.buf_idx;
    size_t reslen = avail <= size ? avail : size;
    std::memcpy(buffer, ctx.buf.data() + ctx.buf_idx, reslen);
    ctx.buf_idx += reslen;
    ctx.cf.trace("response chunk read finish, size: [" + utils::to_string(reslen) + "]");
    return reslen;
}

// callbacks

size_t read_cb(void* buffer, size_t size, void* ctx) /* noexcept */ {
    FetchCtx* fctx = static_cast<FetchCtx*> (ctx);
    try {
        return read(*fctx, buffer, size);
    } catch (const std::exception& e) {
        fctx->append_error(e.what());
        return static_cast<size_t>(-1);
    }
}

size_t write_cb(char* buffer, size_t size, size_t nitems, void* ctx) /* noexcept */ {
    FetchCtx* fctx = static_cast<FetchCtx*> (ctx);
    try {
        return write(*fctx, buffer, size, nitems);
    } catch (const std::exception& e) {
        fctx->append_error(e.what());
        return static_cast<size_t>(-1);
    }
}

size_t headers_cb(char* buffer, size_t size, size_t nitems, void* ctx) /* noexcept */ {
    FetchCtx* fctx = static_cast<FetchCtx*> (ctx);
    try {
        return write_headers(*fctx, buffer, size, nitems);
    } catch (const std::exception& e) {
        fctx->append_error(e.what());
        return static_cast<size_t>(-1);
    }
}

void apply_curlopts(FetchCtx& ctx) {
    // url
    ctx.cf.trace("EVENT_URL " + ctx.cf.remote_version_url);
    setopt_string(ctx.curl, CURLOPT_URL, ctx.cf.remote_version_url);

    // method
    setopt_bool(ctx.curl, CURLOPT_HTTPGET, true);

    // headers
    apply_headers(ctx);

    // callbacks
    setopt_object(ctx.curl, CURLOPT_WRITEDATA, &ctx);
    CURLcode err_wf = curl_easy_setopt(ctx.curl.get(), CURLOPT_WRITEFUNCTION, write_cb);
    if (CURLE_OK != err_wf) {
        throw CheckerException("Error setting option: [CURLOPT_WRITEFUNCTION]");
    }
    setopt_object(ctx.curl, CURLOPT_HEADERDATA, &ctx);
    CURLcode err_hf = curl_easy_setopt(ctx.curl.get(), CURLOPT_HEADERFUNCTION, headers_cb);
    if (CURLE_OK != err_hf) {
        throw CheckerException("Error setting option: [CURLOPT_HEADERFUNCTION]");
    }

    // general behavior options
    if (ctx.cf.curl_force_http_10) {
        CURLcode err = curl_easy_setopt(ctx.curl.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
        if (CURLE_OK != err) {
            throw CheckerException("Error setting option: [CURLOPT_HTTP_VERSION]");
        }
    }
    setopt_bool(ctx.curl, CURLOPT_NOPROGRESS, ctx.cf.curl_noprogress);
    setopt_bool(ctx.curl, CURLOPT_NOSIGNAL, ctx.cf.curl_nosignal);

    // TCP options
    setopt_bool(ctx.curl, CURLOPT_TCP_NODELAY, ctx.cf.curl_tcp_nodelay);
    setopt_uint32(ctx.curl, CURLOPT_CONNECTTIMEOUT_MS, ctx.cf.curl_connecttimeout_millis);

    // HTTP options
    setopt_uint32(ctx.curl, CURLOPT_BUFFERSIZE, ctx.cf.curl_buffersize_bytes);
    setopt_string(ctx.curl, CURLOPT_ACCEPT_ENCODING, ctx.cf.curl_accept_encoding);
    setopt_bool(ctx.curl, CURLOPT_FOLLOWLOCATION, ctx.cf.curl_followlocation);
    setopt_uint32(ctx.curl, CURLOPT_MAXREDIRS, ctx.cf.curl_maxredirs);
    setopt_string(ctx.curl, CURLOPT_USERAGENT, ctx.cf.curl_useragent);

    // throttling options
    setopt_uint32(ctx.curl, CURLOPT_MAX_SEND_SPEED_LARGE, ctx.cf.curl_max_sent_speed_large_bytes_per_second);
    setopt_uint32(ctx.curl, CURLOPT_MAX_RECV_SPEED_LARGE, ctx.cf.curl_max_recv_speed_large_bytes_per_second);

    // SSL options
    setopt_string(ctx.curl, CURLOPT_SSLCERT, ctx.cf.curl_sslcert_filename);
    setopt_string(ctx.curl, CURLOPT_SSLCERTTYPE, ctx.cf.curl_sslcertype);
    setopt_string(ctx.curl, CURLOPT_SSLKEY, ctx.cf.curl_sslkey_filename);
    setopt_string(ctx.curl, CURLOPT_SSLKEYTYPE, ctx.cf.curl_ssl_key_type);
    setopt_string(ctx.curl, CURLOPT_KEYPASSWD, ctx.cf.curl_ssl_keypasswd);
    if (ctx.cf.curl_require_tls) {
        CURLcode err = curl_easy_setopt(ctx.curl.get(), CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
        if (CURLE_OK != err) {
            throw CheckerException("Error setting option: [CURLOPT_SSLVERSION]");
        }
    }
    if (ctx.cf.curl_ssl_verifyhost) {
        setopt_uint32(ctx.curl, CURLOPT_SSL_VERIFYHOST, 2);
    } else {
        setopt_bool(ctx.curl, CURLOPT_SSL_VERIFYHOST, false);
    }
    setopt_bool(ctx.curl, CURLOPT_SSL_VERIFYPEER, ctx.cf.curl_ssl_verifypeer);
    setopt_string(ctx.curl, CURLOPT_CAINFO, ctx.cf.curl_cainfo_filename);
    setopt_string(ctx.curl, CURLOPT_CRLFILE, ctx.cf.curl_crlfile_filename);
    setopt_string(ctx.curl, CURLOPT_SSL_CIPHER_LIST, ctx.cf.curl_ssl_cipher_list);
}

} // namespace

JsonRecord fetchurl(const Config& cf) {
    // init/destroy, this method can be entered only once
    CurlGlobalInitializer global;
    
    // multi
    CurlMultiHolder multi(curl_multi_init());
    setmopt_uint32(multi, CURLMOPT_MAXCONNECTS, cf.curl_max_connects);
    
    // curl
    CurlEasyHolder curl(multi, curl_easy_init());
    FetchCtx ctx(cf, multi, curl);
    apply_curlopts(ctx);
    cf.trace("curl init complete");
    
    // load JSON    
    json_error_t error;
    int flags = JSON_REJECT_DUPLICATES | JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK;
    json_t* json = json_load_callback(read_cb, &ctx, flags, &error);
    if (!json) {
        throw CheckerException(std::string() + "Error parsing JSON:" +
                " text: [" + error.text + "]" +
                " line: [" + utils::to_string(error.line) + "]" +
                " column: [" + utils::to_string(error.column) + "]" +
                " position: [" + utils::to_string(error.position) + "],"
                " callback error: [" + ctx.error + "]");
    }
    return JsonRecord(json);
}

} // namespace

