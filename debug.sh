#!/bin/bash

./make.sh

pushd disk
./update_image.sh
popd

sudo -u enerccio bochs-gdb-a20 -f bochscfg-debug
