#!/bin/bash
# compile kernel
pushd src/kernel
./compile.sh
if [[ $? -ne 0 ]] ; then
    exit 1
fi
popd