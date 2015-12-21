#!/bin/bash

# compile libc for kernel
PATH=$(realpath toolchain/usr/bin):"$PATH" 

rm -rfv $(realpath src/newlib)
mkdir $(realpath src/newlib)

cd build-toolchain
KERNELLIB=$(realpath ../src/kclib)
rm -rfv ${KERNELLIB}/* || true
mkdir ${KERNELLIB}/usr
mkdir ${KERNELLIB}/usr/include
mkdir ${KERNELLIB}/usr/lib

pushd sources/kclib/kclib
git fetch --all
git reset --hard origin/master
make clean
make all PREFIX=${KERNELLIB}/usr CC=x86_64-piko-gcc AR=x86_64-piko-ar
cp -RT include ${KERNELLIB}/usr/include
popd
cd ..

# compile kernel
pushd src/kernel
./compile.sh 
if [[ $? -ne 0 ]] ; then
    exit 1 
fi
popd  
