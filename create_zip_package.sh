#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Error: Version argument required"
    echo "Usage: $0 <version>"
    exit 1
fi

VERSION="$1"
OUTPUT_FILE="InterProcessCourier_${VERSION}.zip"

BINARY_SOURCE="libInterProcessCourier.a"
INCLUDE_SOURCE="include"

TEMP_DIR=$(mktemp -d)
PACKAGE_DIR="$TEMP_DIR/package"

mkdir -p "$PACKAGE_DIR/lib"
mkdir -p "$PACKAGE_DIR/include"

if [ ! -f "$BINARY_SOURCE" ]; then
    echo "Error: Binary file not found at $BINARY_SOURCE"
    rm -rf "$TEMP_DIR"
    exit 1
fi

if [ ! -d "$INCLUDE_SOURCE" ]; then
    echo "Error: Include directory not found at $INCLUDE_SOURCE"
    rm -rf "$TEMP_DIR"
    exit 1
fi

cp "$BINARY_SOURCE" "$PACKAGE_DIR/lib/"
cp -r "$INCLUDE_SOURCE"/* "$PACKAGE_DIR/include/"

CURRENT_DIR=$(pwd)
cd "$PACKAGE_DIR"
zip -r "$CURRENT_DIR/$OUTPUT_FILE" lib/ include/

cd "$CURRENT_DIR"
rm -rf "$TEMP_DIR"

echo "Package created: $CURRENT_DIR/$OUTPUT_FILE"