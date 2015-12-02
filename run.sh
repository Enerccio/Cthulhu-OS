#!/bin/bash

./make.sh

pushd disk
./update_image.sh
popd

sudo -u enerccio bochs -f bochscfg
