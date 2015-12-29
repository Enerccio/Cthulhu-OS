ENTRY(_start)

SECTIONS
{
	. = 0x500;
	
	.mboot BLOCK(4K) : ALIGN(4K)
	{
		*(.mboot)
	}

	. = 1M;

    .ap_data : ALIGN(4K)
    {
       *(.ap_data)
    }
    
    .mp_entry BLOCK(4K) : ALIGN(4K)
    {
        *(.mp_entry)
    }
    
    _mp_entry_end = .;
    _mp_end = .;

	.init BLOCK(4K) : ALIGN(4K)
	{
		*(.init)
	}
	
    _init_end = .;
    
	. =  0xFFFFFFFF80000000 + _init_end;

	.text : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
        _code = .;
        *(.text)
        *(.rodata*)
        *(.stack)
        . = ALIGN(4096);
    }

   .data : AT(ADDR(.data) - 0xFFFFFFFF80000000)
   {
        _data = .;
        *(.data)
        . = ALIGN(4096);
   }
   
   .eh_frame : AT(ADDR(.eh_frame) - 0xFFFFFFFF80000000)
   {
       _ehframe = .;
       *(.eh_frame)
       . = ALIGN(4096);
   }

   .bss : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
   {
       _bss = .;
       *(.bss)
       *(.rodata)
       *(COMMON)
       . = ALIGN(4096);
   }

   _end = .;
   
   .temp_heap_storage BLOCK(40K) : AT(ADDR(.temp_heap_storage)  - 0xFFFFFFFF80000000) {
   		_tempheap = .;
   		*(.temp_heap_storage)
   }

   /DISCARD/ :
   {
        *(.comment)
   }
   
}