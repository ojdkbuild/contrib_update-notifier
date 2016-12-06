/*
 * Copyright 2016 Red Hat, Inc.
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
 * File:   main.cpp
 * Author: alex
 *
 * Created on October 20, 2016, 10:59 PM
 */

#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <string>
#include <vector>

#include <iostream>

#include <popt.h>

#include "CheckerException.hpp"
#include "Config.hpp"
#include "JsonRecord.hpp"
#include "Version.hpp"
#include "jsonio.hpp"
#include "fetchurl.hpp"
#include "platform.hpp"
#include "Tracer.hpp"
#include "utils.hpp"

namespace { // anonymous

class Options {
public:    
    // options list
    char* config;
    int delet;
    int help;
    int usage;
    
    std::string parse_error;
    std::vector<std::string> args;
    struct poptOption table[5];
    poptContext ctx;

    Options(int argc, char** argv) :
    // options initialization
    config(NULL),
    delet(0),
    help(0),
    usage(0),
    
    ctx(NULL) {
        // options table
        struct poptOption tb[] = {
            { "config", 'c', POPT_ARG_STRING, &config, static_cast<int> ('c'), "Path to config file", "config.json"},
            { "delete", 'd', POPT_ARG_NONE,   &delet,  static_cast<int> ('d'), "Delete downloaded file and work directory", NULL},
            { "help",   'h', POPT_ARG_NONE,   &help,   static_cast<int> ('h'), "Show this help message", NULL},
            { "usage", '\0', POPT_ARG_NONE,   &usage,  0, "Display brief usage message", NULL},
            { NULL, 0, 0, NULL, 0, NULL, NULL}
        };
        memcpy(table, tb, sizeof(tb));
        
        { // create context
            ctx = poptGetContext(NULL, argc, const_cast<const char**> (argv), table, POPT_CONTEXT_NO_EXEC);
            if (!ctx) {
                parse_error.append("'poptGetContext' error");
                return;
            }
        }

        { // parse options
            int val;
            while ((val = poptGetNextOpt(ctx)) >= 0);
            if (val < -1) {
                parse_error.append(poptStrerror(val));
                parse_error.append(": ");
                parse_error.append(poptBadOption(ctx, POPT_BADOPTION_NOALIAS));
                return;
            }
        }
        
        { // collect arguments
            const char* ar;
            while(NULL != (ar = poptGetArg(ctx))) {
                args.push_back(std::string(ar));
            }
        }
    }
    
    ~Options() {
        poptFreeContext(ctx);
    }   

private:
    Options(const Options& other);
    
    Options& operator=(const Options& other);
    
};

std::string resolve_config_path(Options& opts, const std::string& appdir) {
    namespace ch = checker;
    if (opts.config) {
        return std::string(opts.config);
    }
    return appdir + "config.json";
}


std::string resolve_version_path(const checker::Config& cf) {
    namespace ch = checker;
    std::string appdata_dir = ch::platform::get_userdata_directory();
    std::string vendor_dir = appdata_dir + cf.vendor_name;
    ch::platform::create_directory(vendor_dir);
    std::string appdir = vendor_dir + "/" + cf.application_name;
    ch::platform::create_directory(appdir);
    return appdir + "/version.json";
}

checker::Version load_local_version(const checker::Config& cf, const std::string& verpath) {
    namespace ch = checker;
    if (ch::platform::file_exists(verpath)) {
        ch::JsonRecord json = ch::read_from_file(verpath, cf.max_json_size_bytes);
        return ch::Version(json);
    } else {
        return ch::Version();
    }
}

void dump_trace(const checker::Config& cf, const checker::Tracer& tr) {
    namespace ch = checker;
    if (!tr.is_enabled()) {
        return;
    }
    try {
        std::string appdatadir = ch::platform::get_userdata_directory();
        std::string path = appdatadir + cf.vendor_name + "/" + cf.application_name + "/trace.json";
        ch::write_to_file(tr.get_json(), path);
        std::cout << "Trace written, path: [" << path << "]" << std::endl;
    } catch (...) {
        //quiet
    }
}

} // namespace


int main(int argc, char** argv) {
    namespace ch = checker;
    
    // parse
    Options opts(argc, argv);
    
    // check invalid options
    if (!opts.parse_error.empty()) {
        std::cerr << "ERROR: " << opts.parse_error << std::endl;
        poptPrintUsage(opts.ctx, stderr, 0);
        return 1;
    }
    
    // check uneeded args
    if (opts.args.size() > 0) {
        std::cerr << "ERROR: invalid arguments specified:";
        for (std::vector<std::string>::iterator it = opts.args.begin(); it != opts.args.end(); ++it) {
            std::cerr << " " << *it;
        }
        std::cerr << std::endl;
        poptPrintUsage(opts.ctx, stderr, 0);
        return 1;
    }
    
    // show help
    if (opts.help) {
        poptPrintHelp(opts.ctx, stderr, 0);
        return 0;
    } else if (opts.usage) {
        poptPrintUsage(opts.ctx, stderr, 0);
        return 0;
    }

    // do work
    try {
        // find out appdir
        std::string exepath = ch::platform::current_executable_path();
        std::string appdir = ch::utils::strip_filename(exepath);

        // load config
        std::string configpath = resolve_config_path(opts, appdir);
        ch::JsonRecord cf_json = ch::read_from_file(configpath, 1 << 15);
        ch::Config cf(cf_json, appdir);

        // init tracing
        ch::Tracer tr(cf.system_trace_enable);
        tr.trace("config loaded from path: [" + configpath + "]");

        // do cleanup and exit (uninstall action)
        if (opts.delet) {
            std::string appdata_dir = ch::platform::get_userdata_directory();
            std::string vendor_dir = appdata_dir + cf.vendor_name;
            ch::platform::delete_file(vendor_dir + "/" + cf.application_name + "/version.json");
            ch::platform::delete_file(vendor_dir + "/" + cf.application_name + "/trace.json");
            ch::platform::delete_directory(vendor_dir + "/" + cf.application_name);
            ch::platform::delete_directory(vendor_dir);
            return 0;
        }
        
        try { // need another try to preserve trace on error
            // load local version
            std::string verpath = resolve_version_path(cf);
            tr.trace("local version path resolved, path: [" + verpath + "]");
            ch::Version local = load_local_version(cf, verpath);
            tr.trace("local version loaded, version number: [" + ch::utils::to_string(local.version_number) + "]");
            tr.trace("EVENT_LOCALVERSION " + ch::utils::to_string(local.version_number));

            // fetch remote version
            ch::JsonRecord remote_json = ch::fetchurl(cf, tr);
            ch::Version remote(remote_json);
            tr.trace("remote version loaded, version number: [" + ch::utils::to_string(remote.version_number) + "]");
            tr.trace("EVENT_REMOTEVERSION " + ch::utils::to_string(remote.version_number));

            // replace local if updated
            if (remote.version_number > local.version_number) {
                ch::write_to_file(remote.to_json(), verpath);
                tr.trace("remote version written, path: [" + verpath + "]");
                tr.trace("EVENT_WRITTEN " + verpath);
                std::cout << ch::platform::current_datetime() <<
                        " INFO: new version descriptor obtained: [" << verpath << "]" << 
                        " old version_number: [" << local.version_number << "]," <<
                        " new version number: [" << remote.version_number << "]" <<
                        std::endl;
            }
            dump_trace(cf, tr);
        } catch(const std::exception& e) {
            tr.trace(std::string() + "ERROR: " + e.what());
            std::cerr << ch::platform::current_datetime() << " ERROR: " << e.what() << std::endl;
            dump_trace(cf, tr);
            return 1;
        }
    } catch(const std::exception& e) {
        std::cerr << ch::platform::current_datetime() << " ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

