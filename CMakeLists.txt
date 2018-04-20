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

cmake_minimum_required ( VERSION 2.8.12 )

# project
include ( ${OJDKBUILD_DIR}/resources/cmake/ojdkbuild_common.cmake )
project ( update_notifier C CXX )

# options
set ( ${PROJECT_NAME}_ICON_PATH "${OJDKBUILD_DIR}/resources/installer/instres/icon.ico" CACHE STRING "Path to icon file" )
set ( ${PROJECT_NAME}_BMP_ICON_PATH "${OJDKBUILD_DIR}/resources/installer/instres/icon.bmp" CACHE STRING "Path to 128x128 bmp icon file" )
set ( ${PROJECT_NAME}_INSTALLER_FEATURE_LEVEL "2" CACHE STRING "MSI Feature 'Level' for 'update_notifier' component" )
set ( ${PROJECT_NAME}_INSTALLER_FEATURE_TITLE "Update Notifier" CACHE STRING "MSI Feature title for 'update_notifier' component" )
set ( ${PROJECT_NAME}_INSTALLER_CHECKER_TASK_NAME "${openjdk_VENDOR_DIRNAME}_jdk_update_checker" CACHE STRING "Name for 'checker' task" ) 
set ( ${PROJECT_NAME}_INSTALLER_CHECKER_SCHEDULE "/sc daily /st 12:00 /ru [LogonUser]" CACHE STRING "Scheduling details for 'checker' task" )
set ( ${PROJECT_NAME}_INSTALLER_NOTIFIER_TASK_NAME "${openjdk_VENDOR_DIRNAME}_jdk_update_notifier" CACHE STRING "Name for 'notifier' task" ) 
set ( ${PROJECT_NAME}_INSTALLER_NOTIFIER_SCHEDULE "/sc onlogon /ru [LogonUser]" CACHE STRING "Scheduling details for 'notifier'" )
set ( ${PROJECT_NAME}_TOOLTIP_LABEL "${openjdk_VENDOR_SHORT} OpenJDK update" CACHE STRING "Notification area tooltip" )
set ( ${PROJECT_NAME}_BALLOON_TITLE_LABEL "${openjdk_VENDOR_SHORT} OpenJDK update available" CACHE STRING "Notification balloon title" )
set ( ${PROJECT_NAME}_UPDATE_TITLE_LABEL "${openjdk_VENDOR_SHORT} OpenJDK update available" CACHE STRING "Update dialog title" )
set ( ${PROJECT_NAME}_CHECK_URL "https://raw.githubusercontent.com/ojdkbuild/ojdkbuild/master/resources/update/java-1.8.0-openjdk.windows.x86_64.version.json" CACHE STRING "URL to check for updates" )
set ( ${PROJECT_NAME}_BROWSER_URL "https://github.com/ojdkbuild/ojdkbuild#downloads-for-windows-x86_64" CACHE STRING "URL to open in browser" )
set ( ${PROJECT_NAME}_CACERTS_PATH "checker.crt" )
set ( ${PROJECT_NAME}_UPDATE_PROCEED_LABEL "To proceed with download and installation please follow a link below" CACHE STRING "'Update' dialog label" )
set ( ${PROJECT_NAME}_BROWSER_ERROR_TITLE_LABEL "${openjdk_VENDOR_SHORT} OpenJDK update error" CACHE STRING "'Error' dialog title" )
set ( ${PROJECT_NAME}_BROWSER_ERROR_TEXT_LABEL "Default web-browser launch error occured, update application will now exit" CACHE STRING "'Error' dialog text" )
set ( ${PROJECT_NAME}_INSTANCE_MUTEX_UUID "7824d8b0-c6d6-47f9-aa39-4498e8e4f498" CACHE STRING "UUID for named mutex" )
set ( ${PROJECT_NAME}_EXE_VERSION "1,${openjdk_MAJOR_VERSION},${openjdk_UPDATE}${openjdk_RPMBUILD},${openjdk_BUILD}" CACHE STRING "EXE version field" )
set ( ${PROJECT_NAME}_FILE_VERSION "1.${openjdk_MAJOR_VERSION}.0.${openjdk_UPDATE}.${openjdk_RPMBUILD}" CACHE STRING "File version field" )
set ( ${PROJECT_NAME}_FILE_DESCRIPTION_CHECKER "${openjdk_VENDOR_SHORT} OpenJDK Update Checker" CACHE STRING "File description field for checker" )
set ( ${PROJECT_NAME}_FILE_DESCRIPTION_NOTIFIER "${openjdk_VENDOR_SHORT} OpenJDK Update Notifier" CACHE STRING "File description field for notifier" )
set ( ${PROJECT_NAME}_COMPANY_NAME_LABEL "${openjdk_VENDOR}" CACHE STRING "Company name version field" )
set ( ${PROJECT_NAME}_PRODUCT_VERSION "${${PROJECT_NAME}_FILE_VERSION}" CACHE STRING "Product version field" )
set ( ${PROJECT_NAME}_SHIPPED_VERSION_NUMBER ${openjdk_MONOTONIC_VERSION_NUMBER} CACHE STRING "Version number of shipped release" )
set ( ${PROJECT_NAME}_CHECKER_MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR}/resources/checker.manifest CACHE STRING "Path to checker manifest" )
set ( ${PROJECT_NAME}_NOTIFIER_MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR}/resources/notifier.manifest CACHE STRING "Path to notifier manifest" )
set ( ${PROJECT_NAME}_VERSION_UPDATE_MESSAGE "This version contains a number of fixes." CACHE STRING "Message for update dialog" )
set ( ${PROJECT_NAME}_NOTIFIER_JAVA_HOME ${CMAKE_CURRENT_BINARY_DIR}/../${openjdk_IMAGE} CACHE STRING "Path to jdk to use" )
# cURL build options
set ( curl_WINXP_COMPAT OFF CACHE BOOL "libCURL WinXP compatibility" )

# static libraries
ojdkbuild_add_external ( ${OJDKBUILD_DIR}/external/zlib )
ojdkbuild_add_external ( ${OJDKBUILD_DIR}/external/jansson )
ojdkbuild_add_external ( ${OJDKBUILD_DIR}/external/openssl )
ojdkbuild_add_external ( ${OJDKBUILD_DIR}/external/curl )
ojdkbuild_add_external ( ${OJDKBUILD_DIR}/external/popt )

# andchor target
add_custom_target ( ${PROJECT_NAME} )

# checker

configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/checker.rc
        ${CMAKE_CURRENT_BINARY_DIR}/checker.rc )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/config.json
        ${CMAKE_CURRENT_BINARY_DIR}/config.json )
configure_file ( ${OJDKBUILD_DIR}/lookaside/ca-certificates/ca-bundle.trust.crt
        ${CMAKE_CURRENT_BINARY_DIR}/checker.crt COPYONLY )

# dependencies
set ( checker_DEPS zlib jansson openssl curl popt )
ojdkbuild_pkg_check_modules ( checker_DEPS_PC REQUIRED checker_DEPS )

# executable
add_executable ( checker
        src/checker.cpp
        src/FileDescriptor.cpp
        src/JsonRecord.cpp
        src/fetchurl.cpp
        src/jsonio.cpp
        src/platform_windows.cpp
        src/Tracer.cpp
        src/transform.cpp
        src/utils.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/checker.rc )

target_include_directories( checker BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${checker_DEPS_PC_INCLUDE_DIRS} )

target_compile_options ( checker PRIVATE /W4 /WX )

target_link_libraries ( checker
        jansson
        popt
        curl
        openssl 
        z
        ws2_32 )

target_compile_options ( checker PRIVATE ${checker_DEPS_PC_CFLAGS} )

set_property ( TARGET checker APPEND_STRING PROPERTY LINK_FLAGS "/manifest:no" )

add_dependencies ( ${PROJECT_NAME} checker )


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
        src/Tracer.cpp
        src/utils.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/notifier.rc )

target_include_directories( notifier BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${notifier_DEPS_PC_INCLUDE_DIRS} )

target_compile_options ( notifier PRIVATE /W4 /WX )

target_link_libraries ( notifier
        jansson
        comctl32 )

set_property ( TARGET notifier APPEND_STRING PROPERTY LINK_FLAGS "/manifest:no" )

add_dependencies ( ${PROJECT_NAME} notifier )

# dist
add_custom_target ( ${PROJECT_NAME}_dist
        COMMAND ${CMAKE_COMMAND} -E make_directory dist
        # checker
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/checker.exe dist
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/resources/checker.vbs dist
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/checker.crt dist
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/config.json dist
        # notifier
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/notifier.exe dist
        COMMAND ${CMAKE_COMMAND} -E copy ${${PROJECT_NAME}_BMP_ICON_PATH} dist
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Creating ${PROJECT_NAME} distribution, path: [${CMAKE_CURRENT_BINARY_DIR}/dist] ..." )
add_dependencies ( ${PROJECT_NAME}_dist ${PROJECT_NAME} )

add_custom_target ( ${PROJECT_NAME}_dist_signed
        # sign binaries
        COMMAND ${${PROJECT_NAME}_NOTIFIER_JAVA_HOME}/bin/jjs.exe ${openjdk_SIGNER_JVM_OPTS}
                ${openjdk_INSTALLER_RESOURCES_PATH}/signer.js
                ${CMAKE_CURRENT_BINARY_DIR}/dist
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Creating ${PROJECT_NAME} signed distribution, path: [${CMAKE_CURRENT_BINARY_DIR}/dist] ..." )
add_dependencies ( ${PROJECT_NAME}_dist_signed ${PROJECT_NAME}_dist image )

# installer
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/installer.xsl
        ${CMAKE_CURRENT_BINARY_DIR}/installer.xsl )

# deployment
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/version.json
        ${CMAKE_CURRENT_BINARY_DIR}/version.json )
