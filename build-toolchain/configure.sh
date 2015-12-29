#!/bin/bash

PREFIX=$(realpath ../toolchain/usr)
SYSROOT=$(realpath ../osroot)
KERNELLIB=$(realpath ../src/newlib)

mkdir build-binutils
mkdir build-gcc

cd build-binutils
rm * -rfv || true
../sources/binutils/binutils-2.24-patched/configure --target=x86_64-fhtagn --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-werror
cd ..

cd build-gcc
rm * -rfv || true
../sources/gcc/gcc-5.1.0-patched/configure --target=x86_64-fhtagn --prefix="$PREFIX" --with-sysroot="$SYSROOT" --enable-languages=c,c++
cd ..

push sources/kclib/kclib
make clean
popd
