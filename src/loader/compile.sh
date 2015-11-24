#!/bin/bash

COMPILE_DIR="../../src/loader/"
BUILDDIR="../../build/pikoloader/"
ASM_ARGS="-felf32"
COMPILER="i686-elf-gcc"
DEFINES=""
COMPILER_ARGS="-ffreestanding -Wall -Wextra -fno-exceptions -std=c99 -g $DEFINES "
LINKER="i686-elf-gcc"
LINKER_ARGS="-ffreestanding -nostdlib -lgcc"

ASM_SOURCES=("loader" "inquire" "../shared/ports")
COMPILE_SOURCES=("entry" "preload"
	"../shared/utils/logger" 
	"../shared/utils/textinput"
	"../shared/utils/rsod"
	"../shared/elf/elf"
	)

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

$LINKER -T ${COMPILE_DIR}loader.ld -o ${BUILDDIR}../piko-loader.img $var $LINKER_ARGS 
rm *.o

popd