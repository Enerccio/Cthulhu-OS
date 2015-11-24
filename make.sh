#!/bin/bash

# compile loader
pushd src/loader
./compile.sh
if [[ $? -ne 0 ]] ; then
    exit 1
fi
popd
# compile kernel
pushd src/kernel
./compile.sh
if [[ $? -ne 0 ]] ; then
    exit 1
fi
popd