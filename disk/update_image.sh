#!/bin/bash

./mount.sh
sudo -u enerccio cp ../build/init initramfs/init
sudo -u enerccio ./mkfs.py initramfs rlyeh
cp ../build/azathoth.img mnt/boot/azathoth.img
cp rlyeh mnt/boot/rlyeh
./umount.sh
