#include <errno.h>
#include <ny/devsys.h>
#include <cthulhu/ct_sys.h>
#include <cthulhu/messages.h>

#include "../rlyeh/rlyeh.h"
#include "../grx/grx.h"
#include "../grx/image.h"
#include "../processes/ipc.h"
#include "../processes/daemons.h"

static volatile bool initramfs_exists = true;

ruint_t allocate_memory(registers_t* r, ruint_t size) {
    cpu_t* cpu = get_current_cput();

    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&__thread_modifier);

    thread_t* ct = cpu->ct;

    mmap_area_t* mmap_area = find_va_hole(ct->parent_process, size, 0x1000);
    if (mmap_area == 0) {
        proc_spinlock_unlock(&__thread_modifier);
        proc_spinlock_unlock(&cpu->__cpu_lock);
        return 0;
    }
    mmap_area->mtype = heap_data;
    allocate(mmap_area->vastart, size, false, false);

    proc_spinlock_unlock(&__thread_modifier);
    proc_spinlock_unlock(&cpu->__cpu_lock);
    return mmap_area->vastart;
}

ruint_t deallocate_memory(registers_t* r, ruint_t from, ruint_t aamount) {
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

	deallocate(mmap_area->vastart, aamount);
	*prev = mmap_area->next;
	free(mmap_area);

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);
	return 0;
}

ruint_t get_tid(registers_t* r) {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&__thread_modifier);

	thread_t* ct = cpu->ct;
	tid_t tid = ct->tId;

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);

	return tid;
}

ruint_t get_pid(registers_t* r) {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&__thread_modifier);

	thread_t* ct = cpu->ct;
	pid_t pid = ct->parent_process->proc_id;

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);

	return pid;
}

ruint_t get_ct_priority(registers_t* r) {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&__thread_modifier);

	thread_t* ct = cpu->ct;
	uint8_t cp = ct->parent_process->priority;

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);

	return cp;
}

ruint_t dev_fb_get_height(registers_t* r) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	return grx_get_height();
}

ruint_t dev_fb_get_width(registers_t* r) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	return grx_get_width();
}

// ipc
int __create_initramfs_process(system_message_t* sm, registers_t* r) {
	if (!initramfs_exists) {
		return ENOENT;
	}

	char* path = (char*)sm->message_contents.cpm.path;
	if (((uintptr_t)path) >= 0xFFFF800000000000 )
		return EINVAL;
	char** argv = sm->message_contents.cpm.argv;
	if (((uintptr_t)argv) >= 0xFFFF800000000000 )
		return EINVAL;
	int argc = sm->message_contents.cpm.argc;
	char** envp = sm->message_contents.cpm.envp;
	if (((uintptr_t)envp) >= 0xFFFF800000000000 )
		return EINVAL;

	path_element_t* pe = get_path(path);
	if (pe == NULL || pe->type == PE_DIR) {
		return ENOENT;
	}

	proc_t* process;
	int rv = create_process_base(get_data(pe->element.file), argc, argv, envp,
			&process, sm->message_contents.cpm.process_priority, r);
	if (rv == 0) {

	}

	return rv;
}

int __create_process(system_message_t* sm, registers_t* r) {
	if (sm->message_contents.cpm.mode == initramfs) {
		return __create_initramfs_process(sm, r);
	}
	return EINVAL;
}

int __system_message(system_message_t* sm, registers_t* r) {
	int rv = EINVAL;
	if (sm->message_type == create_process) {
		rv = __create_process(sm, r);
	}
	return rv;
}

ruint_t sys_send_message(registers_t* r, ruint_t message) {
	if (message >= 0xFFFF800000000000) {
		return (ruint_t)EINVAL;
	}

	message_header_t* mh = (message_header_t*)message;
	if (mh->mtype == system_message) {
		return __system_message((system_message_t*)mh, r);
	}
	return 0;
}

// services
ruint_t get_service_status(registers_t* r, ruint_t sname) {
	const char* name = (const char*) sname;
	if (sname >= 0xFFFF800000000000) {
		return false;
	}
	return daemon_registered(name);
}

// initramfs
ruint_t get_initramfs_entry(registers_t* r, ruint_t p, ruint_t strpnt) {
	if (!initramfs_exists) {
		return E_IFS_INITRAMFS_GONE;
	}
	const char* path = (const char*)(uintptr_t)p;
	initramfs_entry_t* entry = (initramfs_entry_t*)(uintptr_t)strpnt;

	if (p >= 0xFFFF800000000000) {
		return E_EINVAL;
	}
	if (strpnt >= 0xFFFF800000000000) {
		return E_EINVAL;
	}

	path_element_t* pe = get_path(path);

	if (pe->type == PE_DIR) {
		entry->type = et_dir;
		strncpy(entry->name, pe->name, 255);
		entry->num_ent_or_size = array_get_size(pe->element.dir->path_el_array);

		ifs_directory_t* de = ((ifs_directory_t*)entry);
		de->entries = proc_alloc(entry->num_ent_or_size * 8);
		size_t len;
		for (uint32_t i=0; i<entry->num_ent_or_size; i++) {
			path_element_t* child_pe = (path_element_t*)array_get_at(pe->element.dir->path_el_array, i);

			de->entries[i] = proc_alloc((len = strlen(child_pe->name))+1);
			memcpy(de->entries[i], child_pe->name, len+1);
		}
	} else {
		entry->type = et_file;
		strncpy(entry->name, pe->name, 255);
		entry->num_ent_or_size = pe->element.file->size;
		((ifs_file_t*)entry)->file_contents = (char*)
				map_physical_virtual((puint_t)get_data(pe->element.file)
					-ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS),
					(puint_t)((uintptr_t)get_data(pe->element.file))+entry->num_ent_or_size
					-ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS), true);
	}

	return E_IFS_ACTION_SUCCESS;
}
