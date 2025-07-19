import os

from conan.tools.cmake import CMake, CMakeToolchain
from conan.tools.files import copy, collect_libs

from conan import ConanFile


class InterProcessCourier(ConanFile):
    name = "lib-inter-process-courier"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    exports_sources = "CMakeLists.txt", "src/*", "include/*"
    options = {
        "skip_static_analysis": [True, False],
        "skip_compiler_flags": [True, False],
        "skip_tests": [True, False],
        "skip_docs": [True, False],
        "shared": [True, False],
        "fPIC": [True, False]
    }
    default_options = {
        "skip_static_analysis": False,
        "skip_compiler_flags": False,
        "skip_tests": False,
        "skip_docs": False,
        "shared": False,
        "fPIC": True
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build_requirements(self):
        self.tool_requires("doxygen/<host_version>")

    def requirements(self):
        self.requires("doxygen/1.14.0")
        self.requires("gtest/1.16.0")
        self.requires("boost/1.86.0")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["SKIP_STATIC_ANALYSIS"] = self.options.skip_static_analysis
        tc.cache_variables["SKIP_COMPILER_FLAGS"] = self.options.skip_compiler_flags
        tc.cache_variables["SKIP_TESTS"] = self.options.skip_tests
        tc.cache_variables["SKIP_DOCS"] = self.options.skip_docs
        tc.cache_variables["INTER_PROCESS_COURIER_LIB_VERSION"] = self.version
        tc.variables["INTER_PROCESS_COURIER_LIB_VERSION"] = self.version
        tc.generate()

    def build(self):
        print("******* InterProcessCourier build *******")
        print(f"- Version: {self.version}")
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

    def package(self):
        dst = self.package_folder
        src = self.build_folder

        copy(self, pattern="*.hpp", dst=os.path.join(dst, "include/InterProcessCourier"),
             src=os.path.join(src, "include"))
        copy(self, pattern="*.a", dst=os.path.join(dst, "lib"), src=src)
        copy(self, pattern="*.dylib", dst=os.path.join(dst, "lib"), src=src)
        copy(self, pattern="*.so", dst=os.path.join(dst, "lib"), src=src)
        copy(self, pattern="*.so.*", dst=os.path.join(dst, "lib"), src=src)

    def package_info(self):
        self.cpp_info.libs = collect_libs(self)
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib"]
