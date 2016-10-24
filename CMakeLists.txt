
cmake_minimum_required ( VERSION 2.8.12 )

project ( update_checker CXX C )

add_executable ( ${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/src/FileDescriptor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/JsonRecord.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/fetchurl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/jsonio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/jsonio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/platform_linux.cpp )

target_include_directories(${PROJECT_NAME} BEFORE PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src )
