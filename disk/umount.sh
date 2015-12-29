#!/bin/bash

umount /dev/loop1
losetup -d /dev/loop1
losetup -d /dev/loop0 
