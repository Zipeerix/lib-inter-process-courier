#############################################################################
#  InterProcessCourier Copyright (C) 2025  Ziperix                          #
#                                                                           #
#  This program is free software: you can redistribute it and/or modify     #
#  it under the terms of the GNU General Public License as published by     #
#  the Free Software Foundation, either version 3 of the License, or        #
#  (at your option) any later version.                                      #
#                                                                           #
#  This program is distributed in the hope that it will be useful,          #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of           #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
#  GNU General Public License for more details.                             #
#                                                                           #
#  You should have received a copy of the GNU General Public License        #
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.   #
#############################################################################

import os

from conan.tools.cmake import CMake, CMakeToolchain
from conan.tools.files import copy, collect_libs

from conan import ConanFile

required_conan_version = ">=2.0"


class InterProcessCourier(ConanFile):
    name = "lib-inter-process-courier"
    description = "Modern library for client-to-daemon inter-process communication."
    url = "https://github.com/Zipeerix/lib-inter-process-courier"
    homepage = "https://zipeerix.github.io/lib-inter-process-courier/html/"
    author = "Zipeerix (ziperix@icloud.com)"
    license = "GPL-3.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "test/*", "proto/InternalRequests.proto", "Doxyfile.in"
    package_type = "library"
    build_policy = "missing"
    languages = "C++"
    options = {
        "protocol": ["protobuf"],
        "skip_static_analysis": [True, False],
        "skip_compiler_flags": [True, False],
        "skip_tests": [True, False],
        "skip_docs": [True, False],
        "shared": [True, False],
        "fPIC": [True, False]
    }
    default_options = {
        "protocol": "protobuf",
        "skip_static_analysis": False,
        "skip_compiler_flags": False,
        "skip_tests": False,
        "skip_docs": True,
        "shared": False,
        "fPIC": True
    }
    options_description = {
        "protocol": "Defines the protocol used for inter-process communication. Currently only 'protobuf' is supported.",
        "skip_static_analysis": "Skip static analysis checks during the build process.",
        "skip_compiler_flags": "Skip applying custom compiler flags.",
        "skip_tests": "Skip building and running tests.",
        "skip_docs": "Skip building documentation.",
        "shared": "Build shared libraries instead of static libraries.",
        "fPIC": "Position-independent code for shared libraries on Unix-like systems."
    }

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def build_requirements(self):
        if not self.options.skip_docs:
            self.tool_requires("doxygen/<host_version>")

        self.tool_requires("protobuf/<host_version>")

    def requirements(self):
        if not self.options.skip_docs:
            self.requires("doxygen/1.14.0")

        if not self.options.skip_tests:
            self.requires("gtest/1.16.0")

        self.requires("protobuf/5.27.0")
        self.requires("boost/1.88.0")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["SKIP_STATIC_ANALYSIS"] = self.options.skip_static_analysis
        tc.cache_variables["SKIP_COMPILER_FLAGS"] = self.options.skip_compiler_flags
        tc.cache_variables["SKIP_TESTS"] = self.options.skip_tests
        tc.cache_variables["SKIP_DOCS"] = self.options.skip_docs

        tc.cache_variables["INTER_PROCESS_COURIER_LIB_VERSION"] = self.version
        tc.cache_variables["INTER_PROCESS_COURIER_PROTOCOL"] = self.options.protocol

        tc.generate()

    def build(self):
        print("******* InterProcessCourier build *******")
        print(f"- Version: {self.version}")
        print(f"- Protocol: {self.options.protocol}")
        print(f"- Skipping static analysis: {self.options.skip_static_analysis}")
        print(f"- Skipping compiler flags: {self.options.skip_compiler_flags}")
        print(f"- Skipping tests: {self.options.skip_tests}")
        print(f"- Skipping documentation: {self.options.skip_docs}")
        print(f"- Shared library: {self.options.shared}")
        print(f"- fPIC: {self.options.fPIC}")
        print("*****************************************")

        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        if not self.options.skip_tests:
            self.run("./InterProcessCourier_Tests")

    def package(self):
        dst = self.package_folder
        src = self.build_folder

        copy(self, pattern="*.hpp", dst=os.path.join(dst, "include"),
             src=os.path.join(src, "include"))

        for lib_name in ["InterProcessCourier", "InterProcessCourier_InternalRequestsProto"]:
            for extension in ["a", "dylib", "so", ".dll", "so.*"]:
                copy(self, pattern=f"lib{lib_name}.{extension}", dst=os.path.join(dst, "lib"), src=src)

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.libs = collect_libs(self)
