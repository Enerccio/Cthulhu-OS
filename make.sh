#!/bin/bash

set -x

# compile libc for kernel
PATH=$(realpath sysroot/bin):"$PATH"
PATH=$(realpath sysroot/usr/bin):"$PATH" 

rm -rfv $(realpath src/newlib)
mkdir $(realpath src/newlib)

cd build-toolchain/build-newlib-kernel
make -j
make DESTDIR=$(realpath src/newlib) install
cd ../..

# compile kernel
pushd src/kernel
./compile.sh 
if [[ $? -ne 0 ]] ; then
    exit 1 
fi
popd  
