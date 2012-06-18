#include "ukc_mem.h"

#include "omtmem_c.h"

#define TAG_DRV_FRAME 1

void * ukc_malloc(size_t size)
{
	return OMT_Alloc(TAG_DRV_FRAME, size);
}

void ukc_free(void * pData)
{
	OMT_Free(pData);
}


