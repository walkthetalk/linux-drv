#pragma once

/*
 * this header file provide all the interfaces operating on string
 */
#ifndef DRV_KERNEL_SIDE
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "ext_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

void * ukc_malloc(size_t size);
#define UKC_MALLOC(type) ((type *)ukc_malloc(sizeof(type)))
void ukc_free(void *);
#define UKC_FREE(x) ukc_free(x)

#ifdef __cplusplus
}
#endif


