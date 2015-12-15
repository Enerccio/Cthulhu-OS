#!/bin/bash

rm sources/binutils/binutils-2.24-patched -rf || true
cp sources/binutils/binutils-2.24 sources/binutils/binutils-2.24-patched -r
patch -p2 -d sources/binutils/binutils-2.24-patched < patches/binutils.patch
rm sources/gcc/gcc-5.2.0-patched -rf || true
cp sources/gcc/gcc-5.2.0 sources/gcc/gcc-5.2.0-patched -r
patch -p1 -d sources/gcc/gcc-5.2.0-patched < patches/gcc.patch
rm sources/kclib/kclib -rf
pushd sources/kclib
git clone https://github.com/Enerccio/kclib.git
popd
