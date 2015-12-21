BASE_CPPFLAGS  :=-DKERNEL_MODE -D__KCLIB_KERNEL_MODE
BASE_CFLAGS    :=-mcmodel=kernel -ffreestanding -mno-red-zone -std=c99 -Wall -Wextra -Wno-unused-parameter -fno-exceptions -mno-red-zone -mno-mmx -mno-sse -mno-sse2
BASE_LDFLAGS   :=-ffreestanding -mno-red-zone -nostdlib -lgcc -lc -z max-page-size=0x1000
BASE_NASMFLAGS :=-f elf64

