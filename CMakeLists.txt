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
set ( ${PROJECT_NAME}_ICON_PATH "${OJDKBUILD_DIR}/resources/installer/instres/icon.ico" CACHE STRING "Path to icon file" )
set ( ${PROJECT_NAME}_TOOLTIP_LABEL "ojdkbuild update" CACHE STRING "Notification area tooltip" )
set ( ${PROJECT_NAME}_BALLOON_TITLE_LABEL "ojdkbuild update available" CACHE STRING "Notification balloon title" )
set ( ${PROJECT_NAME}_ABOUT_TITLE_LABEL "About ojdkbuild updates" CACHE STRING "'About' dialog title" )
set ( ${PROJECT_NAME}_ABOUT_HEADER_LABEL "About ojdkbuild header" CACHE STRING "'About' dialog header" )
set ( ${PROJECT_NAME}_ABOUT_TEXT_LABEL "About ojdkbuild text" CACHE STRING "'About' dialog text" )
set ( ${PROJECT_NAME}_UPDATE_TITLE_LABEL "Update ojdkbuild" CACHE STRING "'Update' dialog title" )
set ( ${PROJECT_NAME}_BROWSER_URL "https://github.com/ojdkbuild/ojdkbuild#downloads-for-windows-x86_64" CACHE STRING "URL to open in browser" )
set ( ${PROJECT_NAME}_DOWNLOAD_LABEL "Download" CACHE STRING "'Download' menu label" )
set ( ${PROJECT_NAME}_ABOUT_LABEL "About" CACHE STRING "'About' menu label" )
set ( ${PROJECT_NAME}_CANCEL_LABEL "Cancel" CACHE STRING "'Cancel' menu label" )

# static libraries
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/zlib )
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/jansson )
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/openssl )
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/curl )
ojdkbuild_add_subdirectory ( ${OJDKBUILD_DIR}/deps/popt )

# checker

configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/checker.rc
        ${CMAKE_CURRENT_BINARY_DIR}/checker.rc )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/config.json
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json )

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
        src/utils.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/checker.rc )

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


# notifier

# dependencies
set ( notifier_DEPS jansson )
ojdkbuild_pkg_check_modules ( notifier_DEPS_PC REQUIRED notifier_DEPS )

configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/notifier.rc
        ${CMAKE_CURRENT_BINARY_DIR}/notifier.rc )

add_executable ( notifier WIN32
        src/notifier.cpp
        src/FileDescriptor.cpp
        src/JsonRecord.cpp
        src/jsonio.cpp
        src/platform_windows.cpp
        src/utils.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/notifier.rc )

target_include_directories( notifier BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${notifier_DEPS_PC_INCLUDE_DIRS} )

target_link_libraries ( notifier
        lookaside_jansson
        comctl32 )

set_property ( TARGET notifier APPEND_STRING PROPERTY LINK_FLAGS
        "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"" )
