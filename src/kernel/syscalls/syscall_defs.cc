#include <errno.h>
#include <ny/devsys.h>
#include <cthulhu/ct_sys.h>

ruint_t allocate_memory(registers_t* r, ruint_t size) {
    cpu_t* cpu = get_current_cput();

    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&__thread_modifier);

    thread_t* ct = cpu->threads;

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

	thread_t* ct = cpu->threads;
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

	thread_t* ct = cpu->threads;
	tid_t tid = ct->tId;

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);

	return tid;
}

ruint_t get_pid(registers_t* r) {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&__thread_modifier);

	thread_t* ct = cpu->threads;
	pid_t pid = ct->parent_process->proc_id;

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);

	return pid;
}

ruint_t fork(registers_t* r, ruint_t contcall, ruint_t ecptr) {
	ruint_t* ec = (ruint_t*)(uintptr_t)ecptr;

	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&__thread_modifier);

	thread_t* ct = cpu->threads;
	pid_t pid = ct->parent_process->proc_id;

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);

	int errc = fork_process(r, ct->parent_process, ct);
	if (errc != 0) {
		*ec = errc;
	}
	return pid;
}

#include "sys_mmap.h"
ruint_t mem_map(registers_t* r, ruint_t mmap_structure) {
	struct memmap* mmap = (struct memmap*)mmap_structure;


	return 0;
}

#include "../grx/grx.h"
#include "../grx/image.h"

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

ruint_t dev_fb_get_ka(registers_t* r) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	extern uint32_t* local_fb;
	return (ruint_t)(uintptr_t)local_fb;
}

ruint_t dev_fb_get_ba(registers_t* r) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	extern uint8_t* local_fb_changes;
	return (ruint_t)(uintptr_t)local_fb_changes;
}

ruint_t dev_fb_update(registers_t* r) {
	if (daemon_registered(SERVICE_FRAMEBUFFER) && !is_daemon_process(get_current_pid(), SERVICE_FRAMEBUFFER))
		return DS_ERROR_NOT_ALLOWED;
	// TODO: add authorization

	flush_buffer();
}

#include "../rlyeh/rlyeh.h"

static volatile bool initramfs_exists = true;

ruint_t get_initramfs_entry(registers_t* r, ruint_t p, ruint_t strpnt) {
	if (!initramfs_exists) {
		return E_IFS_INITRAMFS_GONE;
	}
	const char* path = (const char*)(uintptr_t)p;
	initramfs_entry_t* entry = (initramfs_entry_t*)(uintptr_t)strpnt;

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

ruint_t initramfs_execve(registers_t* r, ruint_t pathv, ruint_t argvv, ruint_t argcv,
		ruint_t envpv, ruint_t errnov) {
	int* errno = (int*)errnov;

	if (!initramfs_exists) {
		*errno = ENOENT;
		return -1;
	}
	char* path = (char*)pathv;
	char** argv = (char**)argvv;
	int argc = (int)argcv;
	char** envp = (char**)envpv;

	path_element_t* pe = get_path(path);
	if (pe == NULL || pe->type == PE_DIR) {
		*errno = ENOENT;
		return -1;
	}

	int error = sys_execve(get_data(pe->element.file), argc, argv, envp, r);
	if (error != 0) {
		*errno = error;
		return -1;
	}

	return 0;
}
