#!/bin/bash

export PATH=$(realpath toolchain/usr/bin):/usr/local/bin:$PATH
sudo -u enerccio make clean 
sudo -u enerccio make all

pushd disk
./update_image.sh
popd

sudo -u enerccio nm build/piko-kernel.img | grep " T " | awk '{ print $1" "$3 }' > kernel.sym

sudo -u enerccio bochs -f bochscfg.bxrc
