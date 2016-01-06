#include <errno.h>
#include <ny/devsys.h>

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

#include "dev/framebuffer.cc"
