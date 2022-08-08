cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_NAME network)
project(${PROJECT_NAME})

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -g
        -O0
        -march=native
    )
else()
    add_compile_options(
        -O3
        -march=native
        -g
    )
endif()


include("./scripts/fetch_dependencies.cmake")

fetch_dependency("https://github.com/fmtlib/fmt.git;5f774c0aed4af6494b412cd3883e0f09295bd79d")
fetch_dependency("https://github.com/michaelmaniscalco/common.git;master")
fetch_dependency("https://github.com/michaelmaniscalco/work_contract.git;5dceb6de35e068820bf947d0e0886c1527b02e78")

message("Build Type = ${CMAKE_BUILD_TYPE}")

option(NETWORK_BUILD_DEMO "Build tests" ON)

add_subdirectory(src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
