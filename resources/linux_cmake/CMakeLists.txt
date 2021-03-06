# Copyright 2016 Red Hat, Inc.
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

cmake_minimum_required ( VERSION 2.8.11 )

# project
project ( checker C CXX )

set ( ${PROJECT_NAME}_SRCDIR ${CMAKE_CURRENT_LIST_DIR}/../../src )
set ( ${PROJECT_NAME}_CHECK_URL "https://raw.githubusercontent.com/ojdkbuild/scratch/master/rhtemporary/version.json" )
set ( ${PROJECT_NAME}_CACERTS_PATH "/etc/pki/tls/certs/ca-bundle.crt" )
set ( openjdk_VENDOR_DIRNAME "ojdkbuild" )
set ( openjdk_IMAGE "${PROJECT_NAME}" )

# executable
add_executable ( ${PROJECT_NAME}
        ${${PROJECT_NAME}_SRCDIR}/checker.cpp
        ${${PROJECT_NAME}_SRCDIR}/FileDescriptor.cpp
        ${${PROJECT_NAME}_SRCDIR}/JsonRecord.cpp
        ${${PROJECT_NAME}_SRCDIR}/fetchurl.cpp
        ${${PROJECT_NAME}_SRCDIR}/jsonio.cpp
        ${${PROJECT_NAME}_SRCDIR}/platform_linux.cpp
        ${${PROJECT_NAME}_SRCDIR}/Tracer.cpp
        ${${PROJECT_NAME}_SRCDIR}/transform.cpp
        ${${PROJECT_NAME}_SRCDIR}/utils.cpp )

target_link_libraries ( ${PROJECT_NAME} curl jansson popt )

set_property ( TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS "-Wall -Wextra -Werror" )

configure_file ( ${CMAKE_CURRENT_LIST_DIR}/../config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config.json )
