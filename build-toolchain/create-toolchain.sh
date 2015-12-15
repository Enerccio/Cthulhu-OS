#!/bin/bash

SYSROOT=$(realpath ../toolchain)
export PATH="${SYSROOT}/usr/bin:$PATH"

./patch.sh
./configure.sh
./make.sh
