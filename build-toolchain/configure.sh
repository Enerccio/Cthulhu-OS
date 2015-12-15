#!/bin/bash

PREFIX=$(realpath ../sysroot/usr)
SYSROOT=$(realpath ../sysroot)
KERNELLIB=$(realpath ../src/newlib)

cd build-binutils
rm * -rfv || true
../sources/binutils/binutils-2.24-patched/configure --target=x86_64-piko --prefix="$PREFIX" --with-sysroot="/" --disable-werror
cd ..

cd build-gcc
rm * -rfv || true
../sources/gcc/gcc-5.2.0-patched/configure --target=x86_64-piko --prefix="$PREFIX" --with-sysroot="$SYSROOT" --enable-languages=c,c++
cd ..

cd build-newlib
rm * -rfv || true
../sources/newlib/newlib-2.2.0-1-patched/configure --prefix="$PREFIX" --target=x86_64-piko
cd ..

cd build-newlib-kernel
rm * -rfv || true
export CFLAGS_FOR_TARGET="-fPIC -DKERNEL_MODE"
../sources/newlib/newlib-2.2.0-1-patched/configure --prefix="$KERNELLIB" --target=x86_64-piko
export CFLAGS_FOR_TARGET=""
cd ..
