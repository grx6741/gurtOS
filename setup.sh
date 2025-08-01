#!/bin/sh

set -x

# wget https://ftp.gnu.org/gnu/binutils/binutils-2.45.tar.xz
# wget https://ftp.gnu.org/gnu/gcc/gcc-15.1.0/gcc-15.1.0.tar.gz
# 
# mkdir tools
# 
# tar -xvf binutils-2.45.tar.xz
# tar -xzvf gcc-15.1.0.tar.gz
# 
# rm binutils-2.45.tar.xz
# rm gcc-15.1.0.tar.gz
# 
# mkdir -p build-binutils
# cd build-binutils
# 
# ../binutils-2.45/configure --target=i686-elf --prefix="$(pwd)/../tools" --with-sysroot --disable-nls --disable-werror
# make
# make install
# cd ..
# 
# export PATH="$(pwd)/tools/bin:$PATH"
# 
mkdir -p build-gcc
cd build-gcc
../gcc-15.1.0/configure --target=i686-elf --prefix="$(pwd)/../tools" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx
make all-gcc
make all-target-libgcc
make all-target-libstdc++-v3
make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3

# rm -rf build-binutils
# rm -rf build-gcc
