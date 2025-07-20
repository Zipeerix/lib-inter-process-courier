#[[
###########################################################################
#InterProcessCourier Copyright(C) 2'025 Ziperix #
# #
#This program is free software : you can redistribute it and / or modify #
#it under the terms of the GNU General Public License as published by #
#the Free Software Foundation, either version 3 of the License, or #
#(at your option) any later version.#
# #
#This program is distributed in the hope that it will be useful, #
#but WITHOUT ANY WARRANTY; without even the implied warranty of #
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the #
#GNU General Public License for more details.#
# #
#You should have received a copy of the GNU General Public License #
#along with this program.If not, see < https:  // www.gnu.org/licenses/>. #
###########################################################################
]]

cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED INTER_PROCESS_COURIER_LIB_VERSION)
    message(
            FATAL_ERROR
            "INTER_PROCESS_COURIER_LIB_VERSION is required! Please provide it with -DVERSION_ARG=x.y.z"
    )
endif ()

project(InterProcessCourier LANGUAGES CXX)

find_package(protobuf REQUIRED)
find_package(Boost REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/InternalRequests.proto)
add_library(InterProcessCourier_InternalRequestsProto ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(InterProcessCourier_InternalRequestsProto PRIVATE protobuf::protobuf)
target_include_directories(
        InterProcessCourier_InternalRequestsProto
        PRIVATE
        ${Protobuf_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(
        InterProcessCourier
        include/InterProcessCourier/Version.hpp
        include/InterProcessCourier/SyncServer.hpp
        include/InterProcessCourier/SyncClient.hpp
        include/InterProcessCourier/ProtobufTools.hpp
        include/InterProcessCourier/InterProcessCourier.hpp
        include/InterProcessCourier/ThirdPartyFwd.hpp
        src/Version.cpp
        src/ProtobufTools.cpp
        src/SyncServer.cpp
        src/SyncClient.cpp
        src/SyncUnixDomainClient.cpp
        src/SyncUnixDomainServer.cpp)

set_target_properties(
        InterProcessCourier
        PROPERTIES CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS OFF)
target_compile_definitions(
        InterProcessCourier
        PRIVATE -DINTER_PROCESS_COURIER_LIB_VERSION="${INTER_PROCESS_COURIER_LIB_VERSION}")
target_link_libraries(InterProcessCourier InterProcessCourier_InternalRequestsProto boost::boost protobuf::protobuf)
target_include_directories(InterProcessCourier PRIVATE src)
target_include_directories(InterProcessCourier PRIVATE include)
target_include_directories(InterProcessCourier PRIVATE proto)

if (SKIP_COMPILER_FLAGS)
    message("Skipping compiler flags")
else ()
    if (MSVC)
        target_compile_options(InterProcessCourier PUBLIC "/W4")
    else ()
        target_compile_options(
                InterProcessCourier
                PUBLIC "-Wall"
                "-Wextra"
                "-Werror"
                "-Wshadow"
                "-Wundef"
                "-Wcast-align"
                "-Wpointer-arith"
                "-Wwrite-strings"
                "-Wcast-qual"
                "-Wswitch-enum")
    endif ()
endif ()

if (SKIP_STATIC_ANALYSIS)
    message("Skipping static analysis")
else ()
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if (CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set_target_properties(InterProcessCourier PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else ()
        message(WARNING "clang-tidy not found!")
    endif ()
endif ()

if (SKIP_TESTS)
    message("Skipping tests")
else ()
    find_package(GTest REQUIRED)
    add_executable(InterProcessCourier_Tests test/main.cpp test/Version.Tests.cpp)

    target_link_libraries(InterProcessCourier_Tests PRIVATE InterProcessCourier gtest::gtest)

    set_target_properties(
            InterProcessCourier_Tests
            PROPERTIES CXX_STANDARD 23
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS OFF)
    target_compile_definitions(
            InterProcessCourier_Tests
            PRIVATE -DINTER_PROCESS_COURIER_LIB_VERSION="${INTER_PROCESS_COURIER_LIB_VERSION}")

    target_include_directories(InterProcessCourier_Tests PRIVATE src)
    target_include_directories(InterProcessCourier_Tests PRIVATE include)
endif ()

if (SKIP_DOCS)
    message("Skipping documentation generation")
else ()
    find_package(Doxygen QUIET)

    if (DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        add_custom_target(
                docs ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM
                DEPENDS ${DOXYGEN_OUT})

        add_dependencies(InterProcessCourier docs)
    else ()
        message(WARNING "Doxygen not found. Documentation will not be generated.")
    endif ()
endif ()
