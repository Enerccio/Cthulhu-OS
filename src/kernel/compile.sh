#!/bin/bash

PATH=$(realpath ../../sysroot/usr/bin):"$PATH" 
PATH=$(realpath ../../sysroot/bin):"$PATH"

COMPILE_DIR="../../src/kernel/"
BUILDDIR="../../build/pikokernel/"
ASM_ARGS="-f elf64 "
COMPILER="x86_64-piko-gcc"
DEFINES="-DKERNEL_MODE"
COMPILER_ARGS="-ffreestanding -mno-red-zone -g -O0 -std=c99 -Wall -Wextra -fno-exceptions -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -g $DEFINES -I../newlib/usr/x86_64-piko/include"
LINKER="x86_64-piko-gcc"
LINKER_ARGS="-ffreestanding -mno-red-zone -O0 -nostdlib -lgcc -z max-page-size=0x1000 -L../../src/newlib/usr/x86_64-piko/lib/no-red-zone -lc"

ASM_SOURCES=("loader" "entry64" 
	"entry" "ports" "memory/paging")
COMPILE_SOURCES=("kernel" 
	"memory/heap"
	"memory/paging"
	"utils/kstdlib" 
	"utils/logger" 
	"utils/textinput"
	"utils/rsod"
	"newlib/newlib"
	)

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

$LINKER -T ${COMPILE_DIR}loader.ld -o ${BUILDDIR}../piko-kernel.img $var $LINKER_ARGS 

popd