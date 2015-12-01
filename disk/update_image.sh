#!/bin/bash

losetup /dev/loop0 ../disk.img
losetup /dev/loop1 ../disk.img -o 1048576
mount /dev/loop1 mnt
cp ../build/piko-kernel.img mnt/boot/piko-kernel.img
cp piko-initramfs mnt/boot/piko-initramfs
umount /dev/loop1
losetup -d /dev/loop1
losetup -d /dev/loop0 
