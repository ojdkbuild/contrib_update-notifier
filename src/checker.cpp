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
#include "utils.hpp"

namespace { // anonymous

class Options {
public:    
    // options list
    char* config;
    int help;
    int usage;
    
    std::string parse_error;
    std::vector<std::string> args;
    struct poptOption table[4];
    poptContext ctx;

    Options(int argc, char** argv) :
    // options initialization
    config(NULL),
    help(0),
    usage(0),
    
    ctx(NULL) {
        // options table
        struct poptOption tb[] = {
            { "config", 'c', POPT_ARG_STRING, &config, static_cast<int> ('c'), "Path to config file", "config.json"},
            { "help",   'h', POPT_ARG_NONE,   &help,   static_cast<int> ('h'), "Show this help message", NULL},
            { "usage", '\0', POPT_ARG_NONE,   &usage,  0, "Display brief usage message", NULL},
            { NULL, 0, 0, NULL, 0}
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

std::string resolve_config_path(Options& opts) {
    namespace ch = checker;
    if (opts.config) {
        return std::string(opts.config);
    }
    std::string exepath = ch::platform::current_executable_path();
    std::string dirpath = ch::utils::strip_filename(exepath);
    return dirpath + "config.json";
}


std::string resolve_version_path(const checker::Config& cf) {
    namespace ch = checker;
    // std::string appdata_dir = ch::platform::get_userdata_directory(cf);
    // std::string vendor_dir = appdata_dir + "/" + cf.vendor_name;
    // ch::platform::create_directory(vendor_dir);
    // std::string appdir = vendor_dir + "/" + cf.application_name;
    // ch::platform::create_directory(appdir);
    // return appdir + "/" + cf.version_filename;
    std::string exepath = ch::platform::current_executable_path();
    std::string dirpath = ch::utils::strip_filename(exepath);
    return dirpath + "version.json";
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
        // load config
        std::string configpath = resolve_config_path(opts);
        ch::JsonRecord cf_json = ch::read_from_file(configpath, 1 << 15);
        ch::Config cf(cf_json);
        
        // load local version
        std::string verpath = resolve_version_path(cf);
        ch::Version local = load_local_version(cf, verpath);

        // fetch remote version
        ch::JsonRecord remote_json = ch::fetchurl(cf);
        ch::Version remote(remote_json);
        
        // replace local if updated
        if (remote.version_number > local.version_number) {
            ch::write_to_file(remote.to_json(), verpath);
            std::cout << ch::platform::current_datetime() <<
                    " INFO: new version descriptor obtained: [" << verpath << "]" << 
                    " old version_number: [" << local.version_number << "]," <<
                    " new version number: [" << remote.version_number << "]" <<
                    std::endl;
        }
    } catch(const std::exception& e) {
        std::cerr << ch::platform::current_datetime() << " ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

