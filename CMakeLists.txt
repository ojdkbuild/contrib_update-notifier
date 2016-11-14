# Copyright 2016, akashche at redhat.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 2.8.12 )

# project
set ( OJDKBUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/../.. CACHE INTERNAL "" )
include ( ${OJDKBUILD_DIR}/resources/cmake/ojdkbuild_common.cmake )
project ( update_notifier C CXX )

# options

# static libraries
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/zlib )
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/jansson )
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/openssl )
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/curl )
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/popt )

# checker
# dependencies
set ( checker_DEPS zlib_static jansson openssl curl popt )
ojdkbuild_pkg_check_modules ( checker_DEPS_PC REQUIRED checker_DEPS )

# executable
add_executable ( checker
        src/checker.cpp
        src/FileDescriptor.cpp
        src/JsonRecord.cpp
        src/fetchurl.cpp
        src/jsonio.cpp
        src/platform_windows.cpp
        src/utils.cpp )

target_include_directories( checker BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${checker_DEPS_PC_INCLUDE_DIRS} )

target_link_libraries ( checker
        lookaside_jansson
        lookaside_popt
        lookaside_curl
        lookaside_openssl 
        lookaside_zlib
        ws2_32 )

target_compile_options ( checker PRIVATE ${checker_DEPS_PC_CFLAGS} )

# todo: fixme
file ( COPY ${CMAKE_CURRENT_LIST_DIR}/resources/checker_config.json
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

# notifier
add_executable ( notifier WIN32
        src/utils.cpp
        src/notifier.cpp
        src/notifier.rc )

target_include_directories( notifier BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${checker_DEPS_PC_INCLUDE_DIRS} )
