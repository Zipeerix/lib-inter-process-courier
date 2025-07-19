from conan import ConanFile
from conan.tools.cmake import CMake


class ExampleBasic(ConanFile):
    name = "example-basic"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("lib-inter-process-courier/indev")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
