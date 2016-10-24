/* 
 * File:   main.cpp
 * Author: alex
 *
 * Created on October 20, 2016, 10:59 PM
 */

#include <cstdlib>
#include <stdint.h>

#include <popt.h>

namespace { // anonymous



} // namespace


int main(int argc, char** argv) {
    char* configpath;
    
    struct poptOption optionsTable[] = {
        { "config", 'c', POPT_ARG_STRING, &configpath, 0, "path to config file", "config.json"},
        { NULL, '\0', POPT_ARG_INCLUDE_TABLE, poptHelpOptions, 0, "Help options:", NULL},
        { NULL, 0, 0, NULL, 0}
    };
    poptContext optCon = poptGetContext(NULL, argc, const_cast<const char**>(argv), optionsTable, 0);
    while (poptGetNextOpt(optCon) >= 0);
    poptPrintUsage(optCon, stderr, 0);
    
    
    return 0;
}

