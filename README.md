# InterProcessCourier

![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-23-blue)

**InterProcessCourier** is a lightweight and flexible C++23 library for interprocess communication (IPC), optimized for
use with `systemd` and `launchd` daemons communicating with CLI tools. It provides a clean client-server messaging model
but can also be used for general IPC scenarios.

---

## ✨ Features

- 🔌 IPC communication model: systemd/launchd daemon as **server**, CLI tools as **clients**
- 🧩 General-purpose IPC support for broader system designs
- 🛠️ Cross-platform: **Linux** and **macOS**
- 📚 Auto-generated documentation (with [Doxygen](https://www.doxygen.nl/))
- 📁 Practical examples included

---

## 📂 Project Structure

```
.
├── examples/           # Example usage for different IPC scenarios
├── docs/               # Auto-generated Doxygen documentation
│   └── html/index.html
├── include/            # Public headers
├── src/                # Source files
├── LICENSE             # GNU GPL v3 license
├── conanfile.py        # Conan package configuration
└── create_zip_package.sh  # Script to package the library manually
```

---

## 🔧 Building the Library

### 🧰 Requirements

- C++23 compatible compiler (GCC, Clang, etc.)
- [CMake](https://cmake.org/)
- [Conan 2.x](https://docs.conan.io/2/)

### ⚙️ Build Steps

1. Clone the repository and initialize submodules:

   ```bash
   git clone https://github.com/Zipeerix/lib-inter-process-courier.git
   cd lib-inter-process-courier
   git submodule update --init --recursive
   ```

2. Build and create the package using Conan:

   ```bash
   conan create . --version=<version>
   ```

   > 🔧 You can specify a profile and additional Conan options as needed. See [Conan Docs](https://docs.conan.io/2/) for
   more.

   > 💡 For version argument, use ```indev``` or last release's version.

3. To use in other Conan-based projects, simply add:

   ```python
   self.requires("lib-inter-process-courier/<version>")
   ```

---

## ⚙️ Conan Options

The following Conan options are available to customize the build:

```python
options = {
    "skip_static_analysis": [True, False],
    "skip_compiler_flags": [True, False],
    "skip_tests": [True, False],
    "skip_docs": [True, False],
    "shared": [True, False],
    "fPIC": [True, False]
}
```

Relevant options:

- `skip_static_analysis`: Disable `clang-tidy` checks for faster builds
- `skip_compiler_flags`: Avoid adding custom compiler flags (faster/simple builds)
- `skip_tests`: Skip building test targets
- `skip_docs`: Skip generating documentation with Doxygen

---

## 📦 Using Without Conan Integration

If you're not using Conan to manage your dependencies, you can still integrate InterProcessCourier by:

1. Running:

   ```bash
   conan build . --version=<verion>
   ./create_zip_package.sh
   ```

   > 💡 Just like before, use ```indev``` or last release's version.

2. This will generate a `.zip` archive with:
    - Compiled library
    - Header files
    - Documentation

You can then link it manually in your project.

---

## 📖 Documentation

Documentation is auto-generated with every build (unless `skip_docs=True` is set).

👉 [Browse the Docs](https://zipeerix.github.io/lib-inter-process-courier/docs/html/index.html)

---

## 🧪 Examples

Explore the [`examples/`](./examples) folder for practical usage demos, including:

- Daemon/server mode IPC
- CLI client communication
- General message passing scenarios

---

## 📄 License

Licensed under the **GNU General Public License v3.0**. See the [LICENSE](./LICENSE) file for details.

---

## 🤝 Contributing

Contributions, issues, and feature requests are welcome! Feel free to open
an [issue](https://github.com/Zipeerix/lib-inter-process-courier/issues) or submit a pull request.

---

## 💬 Feedback

If you find this project useful or have suggestions for improvement, let us know via GitHub Discussions or Issues!

---
