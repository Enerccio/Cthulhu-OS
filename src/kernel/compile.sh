#!/bin/bash

COMPILE_DIR="../../src/kernel/"
BUILDDIR="../../build/pikokernel/"
ASM_ARGS="-f elf64 "
COMPILER="x86_64-elf-gcc"
DEFINES="-DKERNEL64BIT"
COMPILER_ARGS="-ffreestanding -O2 -std=c99 -Wall -Wextra -fno-exceptions -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -g $DEFINES "
LINKER="x86_64-elf-gcc"
LINKER_ARGS="-ffreestanding -O2 -nostdlib -lgcc -z max-page-size=0x1000"

ASM_SOURCES=("../shared/ports")
COMPILE_SOURCES=("kernel" 
	"../shared/utils/logger" 
	"../shared/utils/textinput"
	"../shared/utils/rsod"
	"../shared/elf/elf")

for source in "${ASM_SOURCES[@]}"
do
	target=$(basename $source)
	nasm ${source}.s -o ${BUILDDIR}${target}.o $ASM_ARGS
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
	var+="${target}.o "
done
for source in "${COMPILE_SOURCES[@]}"
do
	target=$(basename $source)
	var+="${target}.o "
done

$LINKER -T ${COMPILE_DIR}kernel.ld -o ${BUILDDIR}../piko-kernel.img $var $LINKER_ARGS 
rm *.o

popd