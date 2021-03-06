#include <errno.h>
#include <ny/devsys.h>
#include <cthulhu/ct_sys.h>
#include <cthulhu/messages.h>

#include "../rlyeh/rlyeh.h"
#include "../grx/grx.h"
#include "../grx/image.h"
#include "../processes/ipc.h"
#include "../processes/daemons.h"
#include "../processes/scheduler.h"

#define MAX_CHECKED_ELEMENTS 0x512

static volatile bool initramfs_exists = true;
extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);
extern uintptr_t get_active_page();

bool repair_memory(memstate_t state, uint64_t* b, size_t elements, continuation_t* c) {
	// TODO: dispatch crap here
	switch (state) {
	case ms_okay: return true; // sentinel
	case ms_notpresent:
	case ms_einvalid: {
		// TODO: add abort
	} break;
	case ms_cow: {
		// TODO: add cow
	} break;
	case ms_allocondem: {
		for (size_t ix=0; ix<elements; ix++) {
			alloc_info_t ainfo;

			ainfo.from = b[ix];
			ainfo.amount = 0x1000;
			ainfo.finished = false;
			ainfo.exec = false;
			ainfo.aod = false;

			allocate_mem(&ainfo, false, false, get_active_page());
			if (!ainfo.finished) {
				// TODO: add swapper dispatch call here
				c->present = true;
				return false;
			}
		}
		return true;
	} break;
	case ms_swapped: {
		// TODO: add allocondem
	} break;
	}
	return false;
}

bool validate_address(void* address, size_t size, continuation_t* c) {
	size_t found = 0;
	uint64_t localbuf[MAX_CHECKED_ELEMENTS];
	memstate_t state = check_mem_state((uintptr_t)address, size, localbuf, MAX_CHECKED_ELEMENTS, &found);

	if (state != ms_okay) {
		return repair_memory(state, localbuf, found, c);
	}

	return true;
}

bool validate_string(char* string, continuation_t* c) {
	size_t found = 0;
	uint64_t localbuf[MAX_CHECKED_ELEMENTS];

	uintptr_t addr = (uintptr_t)string;
	uintptr_t nextb = ALIGN_UP(addr, 0x1000);

	while (true) {
		memstate_t state = check_mem_state(addr, nextb-addr, localbuf, MAX_CHECKED_ELEMENTS, &found);
		if (state != ms_okay) {
			return repair_memory(state, localbuf, found, c);
		}
		// mstate is fine, check the characters
		char* s = (char*)addr;
		for (size_t i=0; i<nextb-addr; i++) {
			if (s[i] == '\0')
				return true;
		}
		addr += 0x1000;
		nextb += 0x1000;
	}

	return true;
}

bool validate_message(message_t* message, continuation_t* c) {
	bool test = validate_address((void*)message, sizeof(message_t), c);
	if (test)
		return test;
	uint64_t cm = 0;
	uint8_t* mbytes = (uint8_t*)message;
	for (unsigned int i=0; i<sizeof(message_t); i++) {
		cm += mbytes[i];
	}
	return cm % 16 == 0;
}

ruint_t allocate_memory_cont(registers_t* r, continuation_t* c, ruint_t from, ruint_t size,
		ruint_t addr);

ruint_t allocate_memory(registers_t* r, continuation_t* c, ruint_t size) {
    cpu_t* cpu = get_current_cput();

    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&__thread_modifier);

    thread_t* ct = cpu->ct;

    mmap_area_t** _mmap_area = find_va_hole(ct->parent_process, size, 0x1000);
    mmap_area_t* mmap_area = *_mmap_area;
    if (mmap_area == 0) {
        proc_spinlock_unlock(&__thread_modifier);
        proc_spinlock_unlock(&cpu->__cpu_lock);
        c->present = true;
        return 0;
    }
    mmap_area->mtype = heap_data;
    proc_spinlock_unlock(&__thread_modifier);
    proc_spinlock_unlock(&cpu->__cpu_lock);

    return allocate_memory_cont(r, c, mmap_area->vastart, size, mmap_area->vastart);
}

ruint_t allocate_memory_cont(registers_t* r, continuation_t* c, ruint_t from, ruint_t size,
		ruint_t addr) {
	c->continuation = syscalls[SYS_ALLOC_CONT];
	c->_0 = from;
	c->_1 = size;
	c->_2 = addr;

	alloc_info_t ainfo;

	ainfo.amount = size;
	ainfo.from = from;
	ainfo.exec = true;
	ainfo.finished = false;
	ainfo.aod = true;

	allocate_mem(&ainfo, false, false, get_active_page());

	if (!ainfo.finished) {
		c->_0 = ainfo.from;
		c->_1 = ainfo.amount;
		c->_2 = addr;
		c->present = true;
		// TODO: add message to swapper
	}
	return addr;
}

ruint_t deallocate_memory(registers_t* r, continuation_t* c, ruint_t from, ruint_t aamount) {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&__thread_modifier);

	thread_t* ct = cpu->ct;
	mmap_area_t** prev = &ct->parent_process->mem_maps;
	mmap_area_t* mmap_area = ct->parent_process->mem_maps;
	while (mmap_area != NULL) {
		if (mmap_area->vastart == from) {
			if (mmap_area->vaend != from+aamount) {
				// TODO: add sigsegv
				return 1;
			}
			break;
		}
		prev = &mmap_area->next;
		mmap_area = *prev;
	}

	if (mmap_area == NULL || mmap_area->mtype != heap_data) {
		// TODO: add sigsegv
		return 1;
	}

	deallocate(mmap_area->vastart, aamount, get_active_page());
	*prev = mmap_area->next;
	free(mmap_area);

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);
	return 0;
}

ruint_t dev_selfmap_physical(registers_t* r, continuation_t* c, ruint_t _physaddr, ruint_t _size) {
	if (daemon_registered(SERVICE_DDM) && !is_daemon_process(get_current_pid(), SERVICE_DDM))
		return EINVAL;
	size_t size = (size_t)_size;
	puint_t physaddr = (puint_t)_physaddr;

	uintptr_t ptr = map_physical_virtual(&physaddr, physaddr+size, false);
	if (ptr == 0) {
		// TODO: add swapper
		c->_0 = physaddr;
		c->_1 = size-(physaddr-_physaddr);
		c->present = true;
		return 0;
	}
	return (ruint_t)ptr;
}

ruint_t get_pid(registers_t* r, continuation_t* c) {
	return get_current_pid();
}

ruint_t get_ct_priority(registers_t* r, continuation_t* c) {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&__thread_modifier);

	thread_t* ct = cpu->ct;
	uint8_t cp = ct->parent_process->priority;

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);

	return cp;
}

ruint_t dev_fb_get_height(registers_t* r, continuation_t* c) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	return grx_get_height();
}

ruint_t dev_fb_get_width(registers_t* r, continuation_t* c) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	return grx_get_width();
}

// services
ruint_t get_service_status(registers_t* r, continuation_t* c, ruint_t sname) {
	const char* name = (const char*) sname;
	if (!validate_string((void*)name, c))
		return -1;
	return daemon_registered(name);
}

ruint_t register_service(registers_t* r, continuation_t* c, ruint_t sname) {
	const char* name = (const char*) sname;
	if (!validate_string((void*)name, c))
		return -1;
	return register_daemon_service(get_current_pid(), name, false, c);
}

// initramfs
ruint_t get_initramfs_entry(registers_t* r, continuation_t* c, ruint_t p, ruint_t strpnt) {
	if (!initramfs_exists) {
		return E_IFS_INITRAMFS_GONE;
	}

	const char* path = (const char*)(uintptr_t)p;
	initramfs_entry_t* entry = (initramfs_entry_t*)(uintptr_t)strpnt;

	STATIC_ASSERT(sizeof(ifs_directory_t) == sizeof(ifs_file_t));

	if (!validate_string((void*)p, c))
		return E_EINVAL;

	if (!validate_address((void*)strpnt, sizeof(ifs_directory_t), c))
		return E_EINVAL;

	path_element_t* pe = get_path(path);

	if (pe == NULL) {
		return E_EINVAL;
	}

	if (pe->type == PE_DIR) {
		entry->type = et_dir;
		strncpy(entry->name, pe->name, 255);
		entry->num_ent_or_size = array_get_size(pe->element.dir->path_el_array);

		ifs_directory_t* de = ((ifs_directory_t*)entry);
		de->entries = proc_alloc(entry->num_ent_or_size * 8);
		if (de->entries == NULL)
			c->present = true;
			return ENOMEM_INTERNAL;
		size_t len;
		for (uint32_t i=0; i<entry->num_ent_or_size; i++) {
			path_element_t* child_pe = (path_element_t*)array_get_at(pe->element.dir->path_el_array, i);

			de->entries[i] = proc_alloc((len = strlen(child_pe->name))+1);
			if (de->entries[i] == NULL) {
				proc_dealloc((uintptr_t)de->entries);
				c->present = true;
				return ENOMEM_INTERNAL;
			}
			memcpy(de->entries[i], child_pe->name, len+1);
		}
	} else {
		entry->type = et_file;
		strncpy(entry->name, pe->name, 255);
		entry->num_ent_or_size = pe->element.file->size;
		puint_t fe = (puint_t)get_data(pe->element.file)
							-ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS);
		((ifs_file_t*)entry)->file_contents = (char*)
				map_physical_virtual(&fe,
					(puint_t)((uintptr_t)get_data(pe->element.file))+entry->num_ent_or_size
					-ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS), true);
		if (((ifs_file_t*)entry)->file_contents == NULL) {
			c->present = true;
			return ENOMEM_INTERNAL;
		}
	}

	return E_IFS_ACTION_SUCCESS;
}

ruint_t create_process_ivfs(registers_t* r, continuation_t* c, ruint_t _path, ruint_t _argc,
							ruint_t _argv, ruint_t _envp) {
	size_t ix = 0;
	char* path = (char*)_path;
	if (!validate_string(path, c))
		return EINVAL;

	int argc = _argc;

	char** argv = (char**)_argv;
	if (!validate_address((void*)argv, (argc+1)*8, c)) {
		return EINVAL;
	}
	for (ix=0; ix<(size_t)argc; ++ix) {
		if (!validate_string(argv[ix], c))
			return EINVAL;
	}

	char** envp = (char**)_envp;
	ix = 0;
	while (true) {
		if (!validate_address((void*)(envp+ix), 8, c)) {
			return EINVAL;
		}
		if (envp[ix] == NULL)
			break;
		if (!validate_string(envp[ix], c))
			return EINVAL;
	}

	path_element_t* pe = get_path(path);
	if (pe == NULL || pe->type == PE_DIR) {
		return ENOENT;
	}

	proc_t* cp = get_current_process();

	proc_t* process;
	int rv = create_process_base(get_data(pe->element.file), argc, argv, envp,
			&process, 0, r);
	if (rv == 0) {
		process->parent = cp;
	}

	return rv;
}

// Kernel messages
int self_message(uint64_t command, char* data) {
	if (command == GMI_PROCESS_CREATE_STAGE_1) {
		ruint_t cp;
		int error = cp_stage_1((cp_stage1*)data, &cp);
		if (error != 0) {
			// TODO: reply system
		} else
			return error;
	}
	return 0;
}

// IPC
ruint_t sys_send_message(registers_t* r, continuation_t* c, ruint_t _message) {
	message_t* message = (message_t*)_message;
	if (!validate_message(message, c))
		return 0;

	int retval = 0;

	if (message->header.flags.no_target) {
		retval = self_message(message->header.gps_id, message->data);
	}

	if (retval == ENOMEM_INTERNAL) {
		// TODO: call swapper
		c->present = true;
		return ENOMEM_INTERNAL;
	}

	return 0;
}

ruint_t get_empty_message(registers_t* r, continuation_t* c, ruint_t _mp) {
	message_t** mp = (message_t**)_mp;
	if (!validate_address((void*)mp, 8, c)) {
		return EINVAL;
	}
	proc_t* cp = get_current_process();

	proc_spinlock_lock(&cp->__ob_lock);

	for (int i=0; i<MESSAGE_BUFFER_CNT; i++) {
		_message_t* message = &cp->output_buffer[i];
		if (!message->used) {
			message->used = true;
			memset(message->message, 0, sizeof(message_t));
			*mp = message->message;
			proc_spinlock_unlock(&cp->__ob_lock);
			return 0;
		}
	}

	proc_spinlock_unlock(&cp->__ob_lock);
	return ETRYAGAIN;
}

ruint_t sys_recv_message(registers_t* r, continuation_t* c, ruint_t _message) {

}

// mutex

ruint_t __futex_wait(registers_t* r, continuation_t* c, ruint_t _ftx_addr, ruint_t _state) {
	uint32_t* ftx_addr = (uint32_t*)_ftx_addr;
	uint32_t state = (uint32_t)_state;
	if (!validate_address((void*)(ftx_addr), 8, c)) {
		return EINVAL;
	}
	return (ruint_t)futex_wait(r, ftx_addr, state);
}

ruint_t __futex_wake(registers_t* r, continuation_t* c, ruint_t _ftx_addr, ruint_t _num) {
	uint32_t* ftx_addr = (uint32_t*)_ftx_addr;
	int num = (int)_num;
	if (!validate_address((void*)(ftx_addr), 8, c)) {
		return EINVAL;
	}
	return (ruint_t)futex_wake(r, ftx_addr, num);
}

// PCI
#include "../structures/acpi.h"
#include <ny/ny_dman.h>
ruint_t dev_dm_get_pcie_c(registers_t* r, continuation_t* c) {
	if (daemon_registered(SERVICE_DDM) && !is_daemon_process(get_current_pid(), SERVICE_DDM))
		return EINVAL;

	return get_pcie_numcount();
}

ruint_t dev_dm_get_pcie_info(registers_t* r, continuation_t* c, ruint_t _pcistruct) {
	if (daemon_registered(SERVICE_DDM) && !is_daemon_process(get_current_pid(), SERVICE_DDM))
		return EINVAL;

	pci_bus_t* pcistruct = (pci_bus_t*)_pcistruct;
	int64_t pcicount = get_pcie_numcount();
	if (pcicount <= 0) {
		return EINVAL;
	}

	if (!validate_address(pcistruct, sizeof(pci_bus_t)*pcicount, c)) {
		return EINVAL;
	}

	get_pcie_info(pcistruct);
	return 0;
}
