#!/bin/bash

./make.sh

pushd disk
./update_image.sh
popd

sudo -u enerccio nm build/piko-kernel.img | grep " T " | awk '{ print $1" "$3 }' > kernel.sym

sudo -u enerccio bochs -f bochscfg.bxrc
