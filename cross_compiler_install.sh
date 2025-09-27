
# This File will install cross compiler for 32 bit and 64 bit target
# Tested on Ubuntu 20.04, 22.04, 24.04 and Debian 11, 12
# For other Linux distro, you may need to change the package manager command to install prerequisite packages
# Tested on Windows 10/11 WSL2 Ubuntu 20.04, 22.04, 24.04 and Debian 11, 12
# For Windows WSL2, you need to install Windows Terminal from Microsoft Store and use it instead of default WSL terminal
# You can also use Git Bash terminal, but you need to install wget and other prerequisite packages manually
# This script will install the cross compiler in $HOME/opt/cross directory
# You can change the installation directory by changing the PREFIX variable in the script
# After installation, you need to add $HOME/opt/cross/bin to your PATH variable
# You can do this by adding the following line to your ~/.bashrc or ~/.zshrc file:
# export PATH="$HOME/opt/cross/bin:$PATH"
# Then run source ~/.bashrc or source ~/.zshrc to update your PATH variable
# The main GNU FTP Server is very slow, go to find mirror site from here: https://www.gnu.org/prep/ftp.html
# This script installs a cross-compiler for the i686-elf(32 bit Mode) x86_64-elf(64 Bit Mode) target using GCC and Binutils.
# Latt updated: 2024-06-27

# Written by: Baponkar
# Downloaded from: https://github.com/baponkar/KeblaOS
# License: MIT License

#!/bin/bash

# Set the target architecture: 32 or 64
BIT=echo "Enter target architecture (32 or 64): "
read BIT

if [ "$BIT" == "64" ]; then
    echo "Setting up for 64-bit target (x86_64-elf)"
    TARGET=x86_64-elf
elif [ "$BIT" == "32" ]; then
    echo "Setting up for 32-bit target (i686-elf)"
    TARGET=i686-elf
else
    echo "Invalid BIT value. Please set BIT to either 32 or 64."
    exit 1
fi


HOME=$HOME
PREFIX=$HOME/opt/cross
PATH="$PREFIX/bin:$PATH"

# Versions for GCC and Binutils
GCC_VERSION=14.2.0
BINUTILS_VERSION=2.45

# Download directories
SRC_DIR=$HOME/src
BUILD_DIR=$HOME/build

# Ensure required directories exist
mkdir -p $PREFIX $SRC_DIR $BUILD_DIR

# Install prerequisites
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

sudo apt install dosfstools # For creating FAT filesystem images (if needed)
sudo apt install exfatprogs # For creating exFAT filesystem images (if needed)
sudo apt install libelf-dev # For ELF file support
sudo apt install libssl-dev # For SSL support in some GCC versions
sudo apt install wget       # For downloading files
sudo apt install xz-utils   # For extracting .xz files
sudo apt install ovmf       # FOr UEFI RUN
sudo apt install sbsigntool # For Signing Bootloader
sudo apt install mokutil    # For Verifying with password


# Set the base URL for downloading source files
# You can change the mirror if needed from https://www.gnu.org/prep/ftp.html
# BASE_URL=https://ftp.gnu.org/gnu        # Official GNU FTP (Very Slow)
BASE_URL=https://mirrors.hopbox.net/gnu/  # India Mirror which is faster

# Download and extract Binutils
cd $SRC_DIR
if [ ! -f "binutils-$BINUTILS_VERSION.tar.gz" ]; then
    wget --progress=bar:force $BASE_URL/binutils/binutils-$BINUTILS_VERSION.tar.gz
fi
tar -xzf binutils-$BINUTILS_VERSION.tar.gz

# Download and extract GCC
cd $SRC_DIR
if [ ! -f "gcc-$GCC_VERSION.tar.gz" ]; then
    wget --progress=bar:force $BASE_URL/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz
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


if [ "$BIT" == "32" ]; then
    # Testing  Successfully installation
    # i686-elf-gcc --version
    $HOME/opt/cross/bin/i686-elf-gcc --version

    # i686-elf-g++ --version
    $HOME/opt/cross/bin/i686-elf-g++ --version

    # i686-elf-ld --version
    $HOME/opt/cross/bin/i686-elf-ld --version

    # i686-elf-as --version
    $HOME/opt/cross/bin/i686-elf-as --version

    # i686-elf-objdump --version
    $HOME/opt/cross/bin/i686-elf-objdump --version

    # i686-elf-ar --version
    $HOME/opt/cross/bin/i686-elf-ar --version

    # i686-elf-ranlib --version
    $HOME/opt/cross/bin/i686-elf-ranlib --version

    # i686-elf-nm --version
    $HOME/opt/cross/bin/i686-elf-nm --version

    # i686-elf-objcopy --version
    $HOME/opt/cross/bin/i686-elf-objcopy --version

    # i686-elf-strip --version
    $HOME/opt/cross/bin/i686-elf-strip --version
elif [ "$BIT" == "64" ]; then
    # Testing  Successfully installation
    # x86_64-elf-gcc --version
    $HOME/opt/cross/bin/x86_64-elf-gcc --version

    # x86_64-elf-g++ --version
    $HOME/opt/cross/bin/x86_64-elf-g++ --version

    # x86_64-elf-ld --version
    $HOME/opt/cross/bin/x86_64-elf-ld --version

    # x86_64-elf-as --version
    $HOME/opt/cross/bin/x86_64-elf-as --version

    # x86_64-elf-objdump --version
    $HOME/opt/cross/bin/x86_64-elf-objdump --version

    # x86_64-elf-ar --version
    $HOME/opt/cross/bin/x86_64-elf-ar --version

    # x86_64-elf-ranlib --version
    $HOME/opt/cross/bin/x86_64-elf-ranlib --version

    # x86_64-elf-nm --version
    $HOME/opt/cross/bin/x86_64-elf-nm --version

    # x86_64-elf-objcopy --version
    $HOME/opt/cross/bin/x86_64-elf-objcopy --version

    # x86_64-elf-strip --version
    $HOME/opt/cross/bin/x86_64-elf-strip --version
fi


echo "Installation complete! You can now use the cross-compiler by invoking ${TARGET}-gcc and ${TARGET}-g++."

