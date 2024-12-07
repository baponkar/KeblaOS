[![KeblaOS Badge](https://img.shields.io/badge/Kebla-OS-maker?labelColor=red&color=blue)](https://gitlab.com/baponkar/kebla-os)
[![GitHub Badge](https://img.shields.io/badge/Fork-Me-maker?logo=GitHub&logoColor=Blue&labelColor=white&color=blue)
](https://github.com/baponkar/KeblaOS)
[![GitLab Badge](https://img.shields.io/badge/Fork-Me-maker?logo=GitLab&logoColor=Blue&labelColor=white&color=blue)
](https://gitlab.com/baponkar/KeblaOS)
[![Linux Badge](https://img.shields.io/badge/-Linux-maker?logo=linux&logoColor=black&logoSize=auto&labelColor=white&color=blue)
](https://kernel.com)
![C Badge](https://img.shields.io/badge/C-Language-maker?logo=c&logoColor=black&labelColor=white&color=blue)
![x86_32bit Badge](https://img.shields.io/badge/x86-32bit-maker?logo=intel&labelColor=white&color=blue)
![ASM Badge](https://img.shields.io/badge/ASM-Language-maker?logo=assembly&labelColor=white&color=blue)
--------------------------------------------------------------------------------------------------------------------



# System Setup for building a brand new operating system


Linux based OS like Ubuntu, Debian, Kali Linux, Kubuntu etc are good for development. You can also use those OS without installing in your machine by using VirtualBox, VMWire or Windows SubSystem. `GCC` is C compiler, `NASM` is ASM code compiler those two compiler also required as well as some additional packages or softwares.

Bash [Cross Compiler Installer](./code-examples/cross_compiler_install.sh) for Ubuntu.


The following packages are required for following works :


# Set System for cross Compiler

This file have documentation about creating i686-elf cross compiler in Ubuntu/Debian machine.




* Essential Software/Package :
    - `Make` to Automate Building Process by Makefile
    ```
    sudo apt install make
    ```
    - `build-essential` : Provides a collection of necessary packages like GCC (GNU Compiler Collection), make, and other essential tools for compiling C/C++ programs.
    ```
    sudo apt install build-essential
    ```
    - `libgmp3-dev` : This library provides high-precision arithmetic on integers, rational numbers, and floating-point numbers.
    ```
    sudo apt install libgmp3-dev
    ```
    - `libmpc-dev` : MPC (Multiple Precision Complex) is a library for complex number arithmetic with arbitrarily high precision and correct rounding of the result.
    ```
    sudo apt install libmpc-dev
    ```
    - `libmpfr-dev` : MPFR is a C library for multiple-precision floating-point computations with correct rounding.
    ```
    sudo apt install libmpfr-dev
    ```
    - `texinfo` : Texinfo is a documentation system that converts source files into various formats like HTML, PDF, or plain text.
    Many of the tools used in OS development (like GCC, GDB, etc.) have documentation written in Texinfo format, and this package is required for generating and viewing those documents, especially during the build process of GNU tools.
    ```
    sudo apt install texinfo
    ```
    - `Bison` : Bison is a parser generator, part of the GNU Project. It's often used in building compilers or interpreters by converting grammar definitions into C or C++ code.
    ```
    sudo apt install bison
    ```
    - `Flex` : Flex is a fast lexical analyzer generator. It generates programs that perform pattern matching on text, useful for tokenizing input (e.g., source code).
    When building an OS, Flex is often paired with Bison to generate the lexer part of the compiler or interpreter.
    ```
    sudo apt install flex
    ```
    - `GMP`
    ```
    sudo apt install gmp
    ```
    - `GRUB2` for making iso with GRUB2 Bootloader
    ```
    sudo apt install grub2
    ```
    - `NASM` for ASM code compiler
    ```
    sudo apt install nasm
    ```
    - `QEmu`  Virtual machine 
    ```
    sudo apt update

    #Installing Qemu
    sudo apt install qemu qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils

    #Start Services
    sudo systemctl start libvirtd
    sudo systemctl enable libvirtd

    #Verify installation
    qemu-system-x86_64 --version
    ```
    - `GCC` for C,C++,Fortran Code Compiler. We can install gcc by `sudo apt install gcc` but which is system dependent but we want system independent gcc compiler.We can run `./cross_ompiler_install.sh` or following 
    ```bash
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

        cd $(HOME)
        # Ensure required directories exist
        mkdir -p $PREFIX $SRC_DIR $BUILD_DIR

        cd $SRC_DIR
        wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz
        tar -xzf gcc-$GCC_VERSION.tar.gz

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

        $PREFIX/bin/i686-elf-gcc --version
    ```
    - `Linker` (ld) for linking object file We can install linker by `sudo apt install ld` but we need system independent custom linker
    ```bash
    cd $HOME/src
    wget https://ftp.gnu.org/gnu/binutils/binutils-<version>.tar.gz
    tar -xvf binutils-<version>.tar.gz
    cd binutils-<version>

    mkdir -p $HOME/opt/cross
    mkdir build-binutils
    cd build-binutils
    ../configure --target=x86_64-elf --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
    make
    make install

    $(PREFIX)/bin/i686-elf-ld --version
    ```
    - `xxd` to look inside of a binary file
    ```
    sudo apt install xxd
    ```
    - `genisoimage` for generating iso image file
    ```
    sudo apt install genisoimage
    ```

The above process will install the required packages and cross-compiler i686-elf-gcc, i686-elf-ld .



* Reference:
    - [GCC Cross Compiler](https://wiki.osdev.org/GCC_Cross-Compiler)
    



-------------------------------------------------------
*© 2024 KeblaOS Project. All rights reserved.*
