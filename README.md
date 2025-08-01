## Dependencies

On debian, i needed to install the following packages

```console
mtools xorriso grub-efi-amd64-bin grub-pc-bin qemu-system
```

## Building

Build Binutils and GCC for Cross compiling

```console
# Get the tarballs for binutils and Compiler
wget https://ftp.gnu.org/gnu/binutils/binutils-2.45.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-15.1.0/gcc-15.1.0.tar.gz

# Will hold all the required binaries
mkdir tools

# Uncompress the files
tar -xvf binutils-2.45.tar.xz
tar -xzvf gcc-15.1.0.tar.gz

# Delete the tarballs, you don't need them now
rm binutils-2.45.tar.xz
rm gcc-15.1.0.tar.gz

# Create a temporary directory to hold build files. You can delete this later
mkdir build-binutils
cd build-binutils

../binutils-2.45/configure --target=i686-elf --prefix="$(pwd)/../tools" --with-sysroot --disable-nls --disable-werror
make
make install
cd ..

# These Binaries are required for the next Step
export PATH="$(pwd)/tools/bin:$PATH"

mkdir build-gcc
cd build-gcc
../gcc-15.1.0/configure --target=i686-elf --prefix="$(pwd)/../tools" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx
make all-gcc
make all-target-libgcc
make all-target-libstdc++-v3
make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3

# You can delete these directories now
rm -rf build-binutils
rm -rf build-gcc

```
