#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
	char * begin;
	char * next;
	char * end;
} POOL;

POOL * pool_create(size_t size);
void pool_destroy(POOL *p);
size_t pool_available(POOL *p);
void * pool_alloc(POOL *p, size_t size);
void print_pool(POOL *buffer_pool);