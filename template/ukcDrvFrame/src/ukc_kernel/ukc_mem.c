#include "ukc_mem.h"
#include <linux/slab.h>

void * ukc_malloc(size_t size)
{
	return kmalloc(size, GFP_ATOMIC);
}

void ukc_free(void * pData)
{
	kfree(pData);
}


