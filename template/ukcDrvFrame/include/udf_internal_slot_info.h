#pragma once

#include "udf_cfg.h"

#ifdef __cplusplus
#error
extern "C" {
#endif

/*
 * get the hard ware base address of the specific slot's card
 */
void * UDF_FUNC(udf_get_slot_hw_base_addr)(int slot);


#ifdef __cplusplus
}
#endif
