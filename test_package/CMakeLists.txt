cmake_minimum_required(VERSION 3.20)
project(PackageTest CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(lib-inter-process-courier REQUIRED)
find_package(protobuf REQUIRED)

add_executable(InterProcessCourier_PackageTest src/main.cpp)
target_link_libraries(
    InterProcessCourier_PackageTest PRIVATE protobuf::protobuf
                                            lib-inter-process-courier::lib-inter-process-courier)
