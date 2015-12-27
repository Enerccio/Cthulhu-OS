# PIKOKernel OS

## Information
### What is PIKOKernel OS?

PIKOKernel is minimalistic hobbyist OS. It uses it's own libc (KCLib). Its long term goal is
to support user space applications.

### What type of kernel does PIKOKernel OS use? 
PIKOKernel OS uses PIKOKernel as its main kernel, which will be microkernel.

### What is the point of PIKOKernel?

The point is to have fun making a complete OS and then use it as replacement to current OSes. 

### What are PIKOKernel's requirements

At the moment, PIKOKernel only supports x86-64 architecture. Should work on both intel and amd,
however, it was only tested on intel. So far it was only tested on qemu/bochs, I do not recommend
putting it on real hardware yet (not that it can do anything at the moment).

### What is the status of PIKOKernel?

PIKOKernel is under heavy development. So far it has working long mode, paging memory, interrupt handling, symmetrical multiprocessing, interprocessor communication. What needs to be done (and probably more):

* Loading initrd
* Loading init and daemons
* Userspace mode
* Scheduling, Scheduling with SMP support
* Lots and lots of daemons
* Porting user space programs

## Building PIKOKernel OS

Compiling PIKOKernel requires Unix-like environment (was not tested on windows, but will probably require
cygwin).

### PIKOKernel cross compiler

PIKOKernel requires it's own toolchain (x86-64-piko-*). To compile one, these components are required to be 
present at these locations

* binutils version 2.24 in folder `build-toolchain/sources/binutils/binutils-2.24`
* gcc version 5.1.0 in folder `build-toolchain/sources/gcc/gcc-5.1.0`
* latest version of kclib, will be automatically downloaded from github

To build/rebuild toolchain, execute shell script `build-toolchain/create-toolchain.sh` 
in `build-toolchain` directory. After the compilation is done, you should have working C environment 
in `toolchain` directory and user-space clib in `osroot` directory. 

### Compiling kernel 

Simply invoke `make kernel` in projects root directory. You need to have path to `toolchain/usr/bin` in
your path for compiler to find `x86_64-piko-gcc` and `x86_64-piko-as`. Your environment has to provide
`nasm` assembler ([http://www.nasm.us](http://www.nasm.us)) and working shell. 

After compilation is done, you should have `piko-kernel.img` in `build` directory.

### Compiling libpikodev

`libpikodev` is library used by daemons and other system components. You can create it by invoking 
`make pikodev`. 

After compilation is done, you should have `libpikodev.a` and `libpikodev.h` in `build/libpikodev` directory.

## Running PIKOKernel OS

### Run environment
PIKOKernel OS can be run on `qemu` (recommended version 2.5.0, as it was tested on it), `bochs` or real 
hardware (not recommended). 

PIKOKernel OS kernel requires `initrd` in its own file format. You can use provided python script `mkfs.py` to create `initrd`. This script is located in `disk` directory. `mkfs.py` requires `python 2.7`. To use `mkfs.py` you have to provide input directory and output file. Input directory's content will be packed 
into `initrd` output file, which should have name `piko-initramfs`.  

PIKOKernel OS requires `GRUB 2` as its bootloader (since it is mixed 16-32-64bit elf) with `multiboot support`.
If you do not have `GRUB 2` to create new image, you may create 64MB empty disk file and then use `bspatch` to 
patch that empty disk file with `disk.img.patch`. Then you have to mount that image with offset `1048576`. 
For more information check `disk/update_image.sh`.

### `initrd` contents
`initrd` input directory must contain `init` directory containing `init` init file and directory 
`init/daemons` containing daemons to be loaded initially.  

### Running PIKOKernel OS on QEMU
Simply invoke `qemu-system-x86_64 -hdc <yourdiskimagefile> -m 128 -s -smp cores=<numbercores>,threads=<numberthreads>,sockets=<numsockets> -cpu Haswell,+pdpe1gb`

For more information check `debug.sh`.