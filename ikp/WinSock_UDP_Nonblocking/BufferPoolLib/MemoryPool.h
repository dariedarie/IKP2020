#include <string.h>
#include <stdlib.h>

typedef struct pool
{
	char* begin;
	char * next;
	char * end;
} POOL;

POOL * pool_create(size_t size);
void pool_destroy(POOL *p);
size_t pool_available(POOL *p);
void * pool_alloc(POOL *p, size_t size);