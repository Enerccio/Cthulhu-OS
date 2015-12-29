#!/bin/bash

./mount.sh
sudo -u enerccio ./mkfs.py initramfs piko-initramfs
cp ../build/piko-kernel.img mnt/boot/piko-kernel.img
cp piko-initramfs mnt/boot/piko-initramfs
./umount.sh
