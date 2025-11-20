#ifndef HEAP_HPP
#define HEAP_HPP 1

namespace mem::heap {
	void initialise();

    void* malloc(size_t n);
	void* malloc_aligned(size_t n, size_t alignment);
	void* realloc(void* ptr, size_t n);
	void* calloc(size_t n, size_t size);

	void free(void* ptr);
}

#endif /* HEAP_HPP */