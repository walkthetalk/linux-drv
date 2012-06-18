/*
 * driver bottom half extension header file, provide interfaces for online-offline.
 */

#pragma once

#include "udf_cfg.h"

#ifndef DRV_KERNEL_SIDE
#error "it must be kernel side"
#endif

#ifdef __cplusplus
#error
extern "C" {
#endif

// TODO: add content
int UDF_KFUNC(udf_bh_online)(int slot);
void UDF_KFUNC(udf_bh_offline)(int slot);

#ifdef __cplusplus
}
#endif


