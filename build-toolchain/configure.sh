#!/bin/bash

PREFIX=$(realpath ../toolchain/usr)
SYSROOT=$(realpath ../osroot)
KERNELLIB=$(realpath ../src/newlib)

cd build-binutils
rm * -rfv || true
../sources/binutils/binutils-2.24-patched/configure --target=x86_64-piko --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-werror
cd ..

cd build-gcc
rm * -rfv || true
../sources/gcc/gcc-5.2.0-patched/configure --target=x86_64-piko --prefix="$PREFIX" --with-sysroot="$SYSROOT" --enable-languages=c,c++
cd ..

push sources/kclib/kclib
make clean
popd
