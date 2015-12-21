#!/bin/bash

export PATH=/usr/local/bin:$PATH
sudo -u enerccio ./make.sh

pushd disk
./update_image.sh
popd

sudo -u enerccio qemu-system-x86_64 -hdc disk.img -m 128 -s
