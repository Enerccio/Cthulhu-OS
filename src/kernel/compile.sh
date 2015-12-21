#!/bin/bash

PATH=$(realpath ../../toolchain/usr/bin):"$PATH"

COMPILE_DIR="../../src/kernel/"
BUILDDIR="../../build/pikokernel/"
ASM_ARGS="-f elf64"
COMPILER="x86_64-piko-gcc"
DEFINES="-DKERNEL_MODE -D__KCLIB_KERNEL_MODE"
COMPILER_ARGS="-mcmodel=kernel -ffreestanding -mno-red-zone -g -O0 -std=c99 -Wall -Wextra -Wno-unused-parameter -fno-exceptions -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -g $DEFINES -I../kclib/usr/include"
LINKER="x86_64-piko-gcc"
LINKER_ARGS="  -ffreestanding -mno-red-zone -O0 -nostdlib -lgcc -z max-page-size=0x1000 -L../../src/kclib/usr/lib -lc"

ASM_SOURCES=("entry64" 
	"entry" "ports" "memory/paging")
COMPILE_SOURCES=("kernel" 
	"memory/heap"
	"memory/paging"
	"utils/kstdlib" 
	"utils/logger" 
	"utils/textinput"
	"utils/rsod"
	"kclib/kclib"
	)

pushd ${BUILDDIR}	
rm *.o 
popd

for source in "${ASM_SOURCES[@]}"
do
	target=$(basename $source)
	nasm ${source}.s -o ${BUILDDIR}${target}_o.o $ASM_ARGS
done

for source in "${COMPILE_SOURCES[@]}"
do
	target=$(basename $source)
	$COMPILER -c ${source}.c -o ${BUILDDIR}${target}.o $COMPILER_ARGS
done

pushd ${BUILDDIR}
	
	
var=()

for source in "${ASM_SOURCES[@]}"
do
	target=$(basename $source)
	var+="${target}_o.o "
done
for source in "${COMPILE_SOURCES[@]}"
do
	target=$(basename $source)
	var+="${target}.o "
done

$LINKER -T ${COMPILE_DIR}loader.ld -o ${BUILDDIR}../piko-kernel.img $var $LINKER_ARGS || rm ../piko-kernel.img

popd