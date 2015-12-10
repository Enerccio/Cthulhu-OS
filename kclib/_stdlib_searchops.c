
#include <stdlib.h>

#define __VARRAY_ACCESS(idx) (((uint8_t*)base) + (size*idx))

void* bsearch(const void* key, const void* base,
		size_t nmemb, size_t size,
		int (*compar)(const void *, const void *)){
	uint32_t start = 0;
	uint32_t end = nmemb-1;

	while (start<=end){
		uint32_t mid = start + ((end - start) / 2);
		int32_t cmp = compar(__VARRAY_ACCESS(mid), key) == 0;
		if (cmp == 0)
			return __VARRAY_ACCESS(mid);
		else if (cmp < 0) {
			start = mid + 1;
		} else {
			end = mid - 1;
		}
	}

	return NULL;
}

#define __QSORT_SWAP(id1, id2) \
	memmove(*tmp, __VARRAY_ACCESS(id1), size); \
	memmove(__VARRAY_ACCESS(id1), __VARRAY_ACCESS(id2), size); \
	memmove(__VARRAY_ACCESS(id2), *tmp, size)

void __qsort(void* base, size_t size,
		int (*compar)(const void *, const void *),
		uint32_t start, uint32_t end, void** tmp){
	if (start >= end)
		return;
	int64_t pivot_idx = start + (end - start) / 2;

	int64_t i = start;
	int64_t j = end;

	while (i <= j){
		while (compar(__VARRAY_ACCESS(i), __VARRAY_ACCESS(pivot_idx))<0)
			++i;
		while (compar(__VARRAY_ACCESS(j), __VARRAY_ACCESS(pivot_idx))>0)
			--j;
		if (i <= j){
			__QSORT_SWAP(i, j);
			++i;
			--j;
		}
	}

	if (start < j)
		__qsort(base, size, compar, start, (uint32_t)j, tmp);
	if (end > i)
		__qsort(base, size, compar, (uint32_t)i, end, tmp);
}

void qsort(void* base, size_t nmemb, size_t size,
		int (*compar)(const void *, const void *)){
	void* tmp = malloc(size);
	__qsort(base, size, compar, 0, nmemb-1, &tmp);
	free(tmp);
}
