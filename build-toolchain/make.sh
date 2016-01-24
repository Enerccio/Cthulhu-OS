#!/bin/bash

PREFIX=$(realpath ../toolchain/usr)
SYSROOT=$(realpath ../osroot)
KERNELLIB=$(realpath ../src/kclib)

PATH="$PREFIX/bin:$PATH"

rm -rfv ${SYSROOT}/* || true
rm -rfv ${KERNELLIB}/* || true
mkdir ${SYSROOT}/usr
mkdir ${SYSROOT}/usr/lib
mkdir ${SYSROOT}/usr/include
mkdir ${KERNELLIB}/usr
mkdir ${KERNELLIB}/usr/include
mkdir ${KERNELLIB}/usr/lib

cd build-binutils
make
make install
cd ..

rm -rfv temp || true

rm sources/kclib/kclib -rf
pushd sources/kclib
git clone https://github.com/Enerccio/kclib.git
popd

pushd sources/kclib/kclib
cp -RT include ${SYSROOT}/usr/include
cp -RT include ${KERNELLIB}/usr/include
popd

cd build-gcc
make all-gcc all-target-libgcc
make install-gcc install-target-libgcc
cd ..

pushd sources/kclib/kclib
make clean
make kclib PREFIX=${SYSROOT}/usr CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar MODE=user
make crt0 PREFIX=usr SYSROOT=${SYSROOT} MODE=user
popd

pushd sources/kclib/kclib
make clean
make all PREFIX=${KERNELLIB}/usr CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar
popd

cd build-gcc
make all-target-libstdc++-v3
make install-target-libstdc++-v3
cd ..
