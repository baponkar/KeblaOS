#!/bin/bash

# Set variables for the installation
TARGET=i686-elf
HOME = $(HOME)
PREFIX=$(HOME)/opt/cross
PATH="$PREFIX/bin:$PATH"

# Versions for GCC and Binutils
GCC_VERSION=12.2.0
BINUTILS_VERSION=2.40

# Download directories
SRC_DIR=$HOME/src
BUILD_DIR=$HOME/build

# Ensure required directories exist
mkdir -p $PREFIX $SRC_DIR $BUILD_DIR

# Install prerequisites
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

# Download and extract Binutils
cd $SRC_DIR
if [ ! -f "binutils-$BINUTILS_VERSION.tar.gz" ]; then
    wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
fi
tar -xzf binutils-$BINUTILS_VERSION.tar.gz

# Download and extract GCC
cd $SRC_DIR
if [ ! -f "gcc-$GCC_VERSION.tar.gz" ]; then
    wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz
fi
tar -xzf gcc-$GCC_VERSION.tar.gz

# Build and install Binutils
cd $BUILD_DIR
mkdir -p binutils-build && cd binutils-build
$SRC_DIR/binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# Build and install GCC
cd $BUILD_DIR
mkdir -p gcc-build && cd gcc-build
$SRC_DIR/gcc-$GCC_VERSION/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc

# Verify installation
echo "Cross-compiler installed at: $PREFIX"
echo "Adding $PREFIX/bin to your PATH in ~/.bashrc"

# Update PATH
if ! grep -q "$PREFIX/bin" ~/.bashrc; then
    echo "export PATH=\"$PREFIX/bin:\$PATH\"" >> ~/.bashrc
fi

echo "Installation complete! Restart your terminal or run 'source ~/.bashrc' to update PATH."
