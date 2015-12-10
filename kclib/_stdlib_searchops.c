
#include <stdlib.h>

void* bsearch(const void* key, const void* base,
		size_t nmemb, size_t size,
		int (*compar)(const void *, const void *)){
	uint8_t* sf = (uint8_t*)base;
	for (uint32_t start = 0; start < nmemb; start++){
		if (compar(key, (void*) sf)){
			return (void*) sf;
		}
		sf += size;
	}
	return NULL;
}

inline uint32_t __select_pivot(uint32_t s, uint32_t e){
	return (e-s)/2;
}

uint32_t __partition(void* base, size_t size,
		int (*compar)(const void *, const void *), uint32_t start, uint32_t end,
		uint32_t pidx){
	void* pivot = malloc(size);
	void* tmp = malloc(size);

	memmove(pivot, ((uint8_t*)base)+(pidx*size), size);
	uint32_t i = start;
	for (uint32_t j=start; j<end; j++)
		if (compar(((uint8_t*)base)+(j*size), pivot)){
			memmove(tmp, ((uint8_t*)base)+(j*size), size);
			memmove(((uint8_t*)base)+(j*size), ((uint8_t*)base)+(i*size), size);
			memmove(((uint8_t*)base)+(i*size), tmp, size);
			++i;
		}
	memmove(tmp, ((uint8_t*)base)+(i*size), size);
	memmove(((uint8_t*)base)+(i*size), ((uint8_t*)base)+((end-1)*size), size);
	memmove(((uint8_t*)base)+((end-1)*size), tmp, size);

	free(pivot);
	free(tmp);
	return i;
}

void __qsort(void* base, size_t size,
		int (*compar)(const void *, const void *), uint32_t start, uint32_t end){
	if (start >= end-1)
		return;
	uint32_t pivot_idx = __select_pivot(start, end);
	pivot_idx = __partition(base, size, compar, start, end, pivot_idx);
	__qsort(base, size, compar, start, pivot_idx-1);
	__qsort(base, size, compar, pivot_idx+1, end);
}

void qsort(void* base, size_t nmemb, size_t size,
		int (*compar)(const void *, const void *)){
	__qsort(base, size, compar, 0, nmemb);
}
