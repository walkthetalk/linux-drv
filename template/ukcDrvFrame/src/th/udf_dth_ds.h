#pragma once

#include "udf_dth.h"

#include "udf_dbh.h"

#ifdef __cplusplus
extern "C" {
#endif

// UM/US or KM/KS
#ifdef DRV_USR_MODE
#ifndef DRV_KERNEL_SIDE
	#else
		#error "top half must not *DRV_KERNEL_SIDE* in *DRV_USR_MODE*"
	#endif
#else
	#ifndef DRV_KERNEL_SIDE
		#error "top half must be KERNEL_SIDE in KERNEL_MODE"
	#else
	#endif
#endif

typedef struct
{
	struct UDF_TYPE(udf_drv_ds) ds_base; // implemented by user

	int slot_id;
	void * hw_addr;
	unsigned int mem_size;

	UDF_KTYPE(udf_bh_rep_t) bh_rep;
} UDF_TYPE(SUDF_DRV_DATA);

#define GET_ORI_DRV_POINTER(pDataBase) container_of(pDataBase,UDF_TYPE(SUDF_DRV_DATA),ds_base)
#define GET_BH_FROM_DBASE(pDataBase) (GET_ORI_DRV_POINTER(pDataBase)->bh_rep)



#ifdef __cplusplus
}
#endif

