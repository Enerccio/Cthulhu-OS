#!/bin/bash

PATH=$(realpath toolchain/usr/bin):"$PATH" 

rm -rfv $(realpath src/kclib)
mkdir $(realpath src/kclib)

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
make all MODE=kernel DEBREL="$1" PREFIX=${KERNELLIB}/usr CC=x86_64-fhtagn-gcc AR=x86_64-fhtagn-ar
cp -RT include ${KERNELLIB}/usr/include
popd
cd ..

