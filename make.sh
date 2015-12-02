#!/bin/bash

# compile libc for kernel

cd build-newlib-kernel
make all
make DESTDIR=/usr2/fast/projects/pikokernel/pikokernel64/src/newlib/ install
cd ..

# compile kernel
pushd src/kernel
./compile.sh
if [[ $? -ne 0 ]] ; then
    exit 1
fi
popd