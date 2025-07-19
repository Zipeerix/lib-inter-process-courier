#[[
###########################################################################
#  InterProcessCourier Copyright (C) 2025  Ziperix                        #
#                                                                         #
#  This program is free software: you can redistribute it and/or modify   #
#  it under the terms of the GNU General Public License as published by   #
#  the Free Software Foundation, either version 3 of the License, or      #
#  (at your option) any later version.                                    #
#                                                                         #
#  This program is distributed in the hope that it will be useful,        #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of         #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          #
#  GNU General Public License for more details.                           #
#                                                                         #
#  You should have received a copy of the GNU General Public License      #
#  along with this program.  If not, see <https://www.gnu.org/licenses/>. #
###########################################################################
]]

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED INTER_PROCESS_COURIER_LIB_VERSION)
    message(
        FATAL_ERROR
            "INTER_PROCESS_COURIER_LIB_VERSION is required! Please provide it with -DVERSION_ARG=x.y.z"
    )
endif()

project(InterProcessCourier LANGUAGES CXX)

find_package(Boost REQUIRED)

add_library(InterProcessCourier src/test.cpp)

set_target_properties(
    InterProcessCourier
    PROPERTIES CXX_STANDARD 23
               CXX_STANDARD_REQUIRED YES
               CXX_EXTENSIONS OFF)
target_link_libraries(InterProcessCourier boost::boost)
target_include_directories(InterProcessCourier PRIVATE src)

if(SKIP_COMPILER_FLAGS)
    message("Skipping compiler flags")
else()
    if(MSVC)
        target_compile_options(InterProcessCourier PUBLIC "/W4")
    else()
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
    endif()
endif()

if(SKIP_STATIC_ANALYSIS)
    message("Skipping static analysis")
else()
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set_target_properties(InterProcessCourier PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found!")
    endif()
endif()

if(SKIP_TESTS)
    message("Skipping tests")
else()
    find_package(GTest REQUIRED)
    add_executable(InterProcessCourier_Tests test/main.cpp)

    target_link_libraries(InterProcessCourier_Tests PRIVATE InterProcessCourier gtest::gtest)

    set_target_properties(
        InterProcessCourier_Tests
        PROPERTIES CXX_STANDARD 23
                   CXX_STANDARD_REQUIRED YES
                   CXX_EXTENSIONS OFF)

    target_include_directories(InterProcessCourier_Tests PRIVATE src)
endif()
