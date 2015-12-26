[BITS 64]

[GLOBAL __kclib_environ_search_u]
__kclib_environ_search_u:
	xor rax, rax
	ret

[GLOBAL __kclib_open_file_u]
__kclib_open_file_u:
	xor rax, rax
	ret

[GLOBAL __kclib_fstat_u]
__kclib_fstat_u:
	xor rax, rax
	ret

[GLOBAL __kclib_terminate]
__kclib_terminate:
	xor rax, rax
	ret

[GLOBAL __kclib_allocate]
__kclib_allocate:
	xor rax, rax
	ret

[GLOBAL __kclib_deallocate]
__kclib_deallocate:
	xor rax, rax
	ret

[GLOBAL __kclib_open_std_stream]
__kclib_open_std_stream:
	xor rax, rax
	ret

[GLOBAL __kclib_send_data]
__kclib_send_data:
	xor rax, rax
	ret

[GLOBAL __kclib_read_data]
__kclib_read_data:
	xor rax, rax
	ret
