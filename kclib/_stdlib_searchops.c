
#include <stdlib.h>

void* bsearch(const void* key, const void* base,
		size_t nmemb, size_t size,
		int (*compar)(const void *, const void *)){
	uint8_t* sf = (uint8_t*)base;
	for (uint32_t start = 0; start < nmemb; start++){
		if (compar(key, (void*) sf)<0){
			return (void*) sf;
		}
		sf += size;
	}
	return NULL;
}

#define __QSORT_ACCESS(idx) (((uint8_t*)base) + (size*idx))
#define __QSORT_SWAP(id1, id2) \
	memmove(*tmp, __QSORT_ACCESS(id1), size); \
	memmove(__QSORT_ACCESS(id1), __QSORT_ACCESS(id2), size); \
	memmove(__QSORT_ACCESS(id2), *tmp, size)

void __qsort(void* base, size_t size,
		int (*compar)(const void *, const void *),
		uint32_t start, uint32_t end, void** tmp){
	if (start >= end)
		return;
	int64_t pivot_idx = start + (end - start) / 2;

	int64_t i = start;
	int64_t j = end;

	while (i <= j){
		while (compar(__QSORT_ACCESS(i), __QSORT_ACCESS(pivot_idx))<0)
			++i;
		while (compar(__QSORT_ACCESS(j), __QSORT_ACCESS(pivot_idx))>0)
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
