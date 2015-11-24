#!/bin/bash

# compile loader
pushd src/loader
sudo -u enerccio ./compile.sh
if [[ $? -ne 0 ]] ; then
    exit 1
fi
popd
# compile kernel
pushd src/kernel
sudo -u enerccio ./compile.sh
if [[ $? -ne 0 ]] ; then
    exit 1
fi
popd

pushd disk
./update_image.sh
popd

sudo -u enerccio bochs-gdb-a20 -f bochscfg-debug
