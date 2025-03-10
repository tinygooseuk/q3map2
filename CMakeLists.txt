cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(Q3Map2)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
# Enable Debug by default, can be changed with -DCMAKE_BUILD_TYPE=Release
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

#-----------------------------------------------------------------------
# Version
#-----------------------------------------------------------------------

set(WASI_SDK_PATH "/Users/joe/wasi-sdk")

# CMake 3.0+ would allow this in project()
set(NetRadiant_VERSION_MAJOR 1)
set(NetRadiant_VERSION_MINOR 5)
set(NetRadiant_VERSION_PATCH 0)
set(NetRadiant_VERSION "${NetRadiant_VERSION_MAJOR}.${NetRadiant_VERSION_MINOR}.${NetRadiant_VERSION_PATCH}")


#-----------------------------------------------------------------------
# Language standard
#-----------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# For some reason the above flags don't really work...
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR CMAKE_COMPILER_IS_GNUCXX)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(--std=c++${CMAKE_CXX_STANDARD} STD_CXX)
    if (STD_CXX)
        list(APPEND CMAKE_CXX_FLAGS --std=c++${CMAKE_CXX_STANDARD})
    else ()
        message(SEND_ERROR "Requires C++${CMAKE_CXX_STANDARD} or better")
    endif ()
else ()
    message(WARNING "Unrecognized compiler: ${CMAKE_CXX_COMPILER_ID}, make sure it supports C++${CMAKE_CXX_STANDARD}")
endif ()

#-----------------------------------------------------------------------
# Flags
#-----------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")  # Ensures output is an HTML file
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing -fno-common")
elseif(CMAKE_SYSTEM_NAME STREQUAL "WASI")
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm")  # Ensures output is an HTML file
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing -fno-common")
    set(CMAKE_C_COMPILER "${WASI_SDK_PATH}/bin/clang")
    set(CMAKE_EXE_LINKER_FLAGS "--sysroot=${WASI_SDK_PATH}/share/wasi-sysroot -Wl,--export-all -Wl,--no-entry")

else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing -fcommon")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -fno-exceptions -fno-rtti")
set(CMAKE_POSITION_INDEPENDENT_CODE 1)

#-----------------------------------------------------------------------
# Defs
#-----------------------------------------------------------------------

add_definitions(-DRADIANT_VERSION="${NetRadiant_VERSION}")
add_definitions(-DRADIANT_MAJOR_VERSION="${NetRadiant_VERSION_MAJOR}")
add_definitions(-DRADIANT_MINOR_VERSION="${NetRadiant_VERSION_MINOR}")

add_definitions(-DRADIANT_ABOUTMSG="${NetRadiant_ABOUT}")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D_DEBUG=1)
endif ()

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/libs")

#-----------------------------------------------------------------------
# Libraries
#-----------------------------------------------------------------------

add_subdirectory(libs)

#-----------------------------------------------------------------------
# Tools
#-----------------------------------------------------------------------

add_subdirectory(tools)
