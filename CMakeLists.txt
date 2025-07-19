cmake_minimum_required(VERSION 3.20)
project(InterProcessCourier VERSION 0.1.0 LANGUAGES CXX)

find_package(Boost REQUIRED)

add_library(InterProcessCourier
        src/test.cpp)
set_target_properties(
        InterProcessCourier
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS OFF)
target_link_libraries(InterProcessCourier boost::boost)
target_include_directories(InterProcessCourier PRIVATE src)

if (SKIP_COMPILER_FLAGS)
    message("Skipping compiler flags")
else ()
    if (MSVC)
        target_compile_options(InterProcessCourier PUBLIC "/W4")
    else ()
        target_compile_options(
                InterProcessCourier
                PUBLIC
                "-Wall"
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

if (SKIP_TESTS)
    message("Skipping tests")
else ()
    find_package(GTest REQUIRED)
    add_executable(InterProcessCourier_Tests test/main.cpp)

    target_link_libraries(InterProcessCourier_Tests PRIVATE InterProcessCourier gtest::gtest)

    set_target_properties(
            InterProcessCourier_Tests
            PROPERTIES CXX_STANDARD 23
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS OFF)

    target_include_directories(InterProcessCourier_Tests PRIVATE src)
endif ()
