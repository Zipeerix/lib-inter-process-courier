#!/bin/bash

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'
DIRS=("src" "include" "test")

if [ ! -f ".clang-format" ]; then
    echo "Error: .clang-format file not found in current directory"
    exit 1
fi

if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed or not in PATH"
    exit 1
fi

if ! command -v cmake-format &> /dev/null; then
    echo "Warning: cmake-format is not installed or not in PATH"
    echo "CMakeLists.txt files will be skipped"
    CMAKE_FORMAT_AVAILABLE=false
else
    CMAKE_FORMAT_AVAILABLE=true
fi

echo -e "${BLUE}Applying formatting to src/ and include/ directories${NC}"
echo -e "${BLUE}Using .clang-format from: $(pwd)/.clang-format${NC}"
if [ "$CMAKE_FORMAT_AVAILABLE" = true ]; then
    echo -e "${BLUE}cmake-format will be applied to CMakeLists.txt in working directory${NC}"
fi
echo ""

for dir in "${DIRS[@]}"; do
    if [ -d "$dir" ]; then
        echo -e "${YELLOW}Processing directory: $dir/${NC}"
        find "$dir" -type f \( \
            -name "*.c" -o \
            -name "*.cpp" -o \
            -name "*.cc" -o \
            -name "*.cxx" -o \
            -name "*.c++" -o \
            -name "*.h" -o \
            -name "*.hpp" -o \
            -name "*.hh" -o \
            -name "*.hxx" -o \
            -name "*.h++" \
        \) | while read -r file; do
            echo -e "${GREEN}Formatting (clang):${NC} $file"
            clang-format -i "$file"
        done
    else
        echo -e "${YELLOW}Directory $dir/ not found, skipping${NC}"
    fi
done

if [ "$CMAKE_FORMAT_AVAILABLE" = true ]; then
    if [ -f "CMakeLists.txt" ]; then
        echo -e "${GREEN}Formatting (cmake):${NC} CMakeLists.txt"
        echo -e "${GREEN}Formatting (cmake):${NC} test_package/CMakeLists.txt"
        cmake-format -i "CMakeLists.txt"
        cmake-format -i "test_package/CMakeLists.txt"
    else
        echo -e "${YELLOW}CMakeLists.txt not found in working directory${NC}"
    fi
fi

echo ""
echo -e "${BLUE}Formatting complete!${NC}"