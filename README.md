# Cthulhu OS

>  “Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn.
> 
>  In his house at R'lyeh dead Cthulhu waits dreaming.”
>   ― H.P. Lovecraft, The Call of Cthulhu 

## Information
### What is Cthulhu OS?

Cthulhu OS is minimalistic hobbyist OS. It uses it's own libc (KCLib). Its long term goal is
to support user space applications.

### What type of kernel does Cthulhu OS use? 
Cthulhu OS uses Azathoth as its main kernel, which will be microkernel.

### What is the point of Cthulhu OS?

The point is to have fun making a complete OS and then use it as replacement to current OSes. 

### What are Cthulhu OS's requirements

At the moment, Cthulhu OS only supports x86-64 architecture. Should work on both Intel and AMD,
however, it was only tested on Intel. So far it was only tested on qemu, I do not recommend
putting it on real hardware yet (not that it can do anything at the moment).

### What is the status of Cthulhu OS?

Cthulhu OS is under heavy development. 

#### Working features
* boot code
* framebuffer support, printing text
* paging, segmentation, memory management
* initramfs
* user space
* scheduler
* elf loader (no dynamic linking yet)

#### Stuff to do
* Lots and lots of daemons
* Porting user space programs
* Dynamic linking, shared libraries

## Building Cthulhu OS

Compiling Cthulhu OS requires Unix-like environment (was not tested on windows, but will probably require
cygwin).

### Cthulhu OS cross compiler

Cthulhu OS requires it's own toolchain (x86-64-fhtagn-*). To compile one, these components are required to be 
present at these locations

* binutils version 2.24 in folder `build-toolchain/sources/binutils/binutils-2.24`
* gcc version 5.1.0 in folder `build-toolchain/sources/gcc/gcc-5.1.0`
* latest version of kclib, will be automatically downloaded from github

To build/rebuild toolchain, execute shell script `build-toolchain/create-toolchain.sh` 
in `build-toolchain` directory. After the compilation is done, you should have working C environment 
in `toolchain` directory and user-space clib in `osroot` directory. 

### Compiling kernel 

Simply invoke `make kernel` in projects root directory. You need to have path to `toolchain/usr/bin` in
your path for compiler to find `x86_64-fhtagn-gcc` and `x86_64-fhtagn-as`. Your environment has to provide
`nasm` assembler ([http://www.nasm.us](http://www.nasm.us)) and working shell. 

After compilation is done, you should have `azathoth.img` in `build` directory.

### Compiling libcthulhu

`libcthulhu` is library used by all programs in Cthulhu-OS. Internally, it is used by some libc syscalls, therefore it is necessary to be always present.  
`make cthulhu`. 

After compilation is done, you should have `libcthulhu.a` and `cthulhu` directory in  `$SYSROOT$/usr/include` directory.

### Compiling libnyarlathotep

`libnyarlathotep` is library used by daemons and other system components. You can create it by invoking 
`make nyarlathotep`. 

After compilation is done, you should have `libnyarlathotep.a` and `ny` directory in  `$SYSROOT$/usr/include` directory.

## Running Cthulhu OS

### Run environment
Cthulhu OS can be run on `qemu` (recommended version 2.5.0, as it was tested on it), `bochs` or real 
hardware (not recommended). 

Cthulhu OS's kernel Azatoth requires `initrd` in its own file format. You can use provided python script `mkfs.py` to create `initrd`. This script is located in `disk` directory. `mkfs.py` requires `python 2.7`. To use `mkfs.py` you have to provide input directory and output file. Input directory's content will be packed 
into `initrd` output file, which should have name `rlyeh`.  

Cthulhu OS requires `GRUB 2` as its bootloader (since it is mixed 16-32-64bit elf) with `multiboot support`.
If you do not have `GRUB 2` to create new image, you may create 64MB empty disk file and then use `bspatch` to 
patch that empty disk file with `disk.img.patch`. Then you have to mount that image with offset `1048576`. 
For more information check `disk/update_image.sh`.

Sample grub configuration is in directory `boot`, which you can copy into your boot partition.

### `initrd` contents
`initrd` 

#### `init`

`initrd` input directory must contain `sys` directory containing `init` init file and directory 
`sys/daemons` containing daemons to be loaded initially.  

### Running Cthulhu OS on QEMU
Simply invoke `qemu-system-x86_64 -hdc <yourdiskimagefile> -m 128 -s -smp cores=<numbercores>,threads=<numberthreads>,sockets=<numsockets> -cpu Haswell,+pdpe1gb`

For more information check `debug.sh`.