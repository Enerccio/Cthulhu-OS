#!/bin/bash

PREFIX=$(realpath ../sysroot/usr)
SYSROOT=$(realpath ../sysroot)
KERNELLIB=$(realpath ../src/newlib)
OLDP=$PATH

rm -rfv $SYSROOT/* || true
mkdir ${SYSROOT}/usr
mkdir ${SYSROOT}/usr/include

cd build-binutils
make
make install
cd ..

rm -rfv temp || true
mkdir temp
cd temp

ln -s /usr/bin/x86_64-elf-ar x86_64-piko-ar
ln -s /usr/bin/x86_64-elf-as x86_64-piko-as
ln -s /usr/bin/x86_64-elf-gcc x86_64-piko-gcc
ln -s /usr/bin/x86_64-elf-gcc x86_64-piko-cc
ln -s /usr/bin/x86_64-elf-ranlib x86_64-piko-ranlib

PATH=$(pwd):$PATH
cd ..

cd build-newlib
export EX_CPPFLAGS_FOR_TARGET=" "
export EX_LFLAGS_FOR_TARGET=" "
make all
make DESTDIR=$SYSROOT install
cd ..
cp -ar ${SYSROOT}/${SYSROOT}/usr/x86_64-piko/* ${SYSROOT}/usr/

cd build-newlib-kernel
export EX_CPPFLAGS_FOR_TARGET=" -mcmodel=kernel -DKERNEL_MODE"
export EX_LFLAGS_FOR_TARGET=" "
make all
make DESTDIR=$KERNELLIB install
cd ..

PATH=$OLDP

cd build-gcc
make all-gcc all-target-libgcc
make install-gcc install-target-libgcc
make all-target-libstdc++-v3
make install-target-libstdc++-v3

/bin/cp -ar ${SYSROOT}/usr/lib/gcc ${SYSROOT}/lib/ -rf
cp ${SYSROOT}/bin/x86_64-piko-gcc ${SYSROOT}/bin/x86_64-piko-cc
cd ..
