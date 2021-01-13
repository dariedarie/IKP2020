#include "MemoryPool.h"

POOL * pool_create(size_t size) {
	POOL * p = (POOL*)malloc(size + sizeof(POOL));
	p->begin = (char*)&p[1];
	p->next = (char*)&p[1];
	p->end = p->next + size;
	return p;
}

void pool_destroy(POOL *p) {
	free(p);
}

size_t pool_available(POOL *p) {
	return p->end - p->next;
}

void * pool_alloc(POOL *p, size_t size) {
	if (pool_available(p) < size) return NULL;
	void *mem = (void*)p->next;
	p->next += size;
	return mem;
}

void print_pool(POOL *buffer_pool) {
	printf("BUFFER POOL:");
	for (char *c = buffer_pool->begin; c != buffer_pool->next; c++)
	{
		printf("%c", *c);
	}
	printf("\n");
}