#!/bin/bash

set -x

# compile libc for kernel

PATH=$(realpath sysroot/usr/bin):"$PATH" 
PATH=$(realpath sysroot/bin):"$PATH"

cd build-toolchain/build-newlib-kernel
make all
make DESTDIR=$(realpath src/newlib) install
cd ../..

# compile kernel
pushd src/kernel
./compile.sh
if [[ $? -ne 0 ]] ; then
    exit 1 
fi
popd 