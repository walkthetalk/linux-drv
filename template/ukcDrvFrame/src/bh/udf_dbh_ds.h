#pragma once

#include UDF_KDRV_DS_HDR_FILE_NAME

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DRV_KERNEL_SIDE
#error "this is only used in kernel code"
#endif

typedef struct UDF_KTYPE(udf_drv_bh_data)
{
	struct UDF_KTYPE(udf_drv_ds) ds_base;
	int slot;
	void * hw_addr;
} UDF_KTYPE(SUDF_KDRV_DATA);

#define GET_ORI_KDRV_POINTER(pData) container_of(pData,UDF_KTYPE(SUDF_KDRV_DATA),ds_base)



#ifdef __cplusplus
}
#endif

