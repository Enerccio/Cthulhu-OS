#!/bin/bash

losetup /dev/loop0 ../disk.img
losetup /dev/loop1 ../disk.img -o 1048576
mount /dev/loop1 mnt
