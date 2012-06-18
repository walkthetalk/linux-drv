/*
 * upper level user application use this file to control device, (need link with
 * the library generated when compiling driver)
 */
#pragma once

#include "kdrv_op_set.h"
/*
 * TODO: change the file name to *UDF_KDRV_IF_DS_HDR_FILE_NAME*
 */
#include "drv_if_ds_cpmk.h"



#ifdef __cplusplus
extern "C" {
#endif

/*
 * get the operation set from low level driver
 */
/*
 * get the operation set from low level driver
 * TODO: change the function's name
 * NOTE: the name must be *get_drv_op_set_***k_fn*, in which the only
 * part you can/need change is *cpm*; change it to your own identificaion,
 * please see macro *UDF_TOKEN_SUBFIX* in your cfg file.
 */
int get_drv_op_set_cpmk_fn(udf_kdrv_op_set_t * rSet);

/*
 * declare all user-defined interfaces used by upper level driver.
 * NOTE: all of them has a subfix *_kernel*. (in where it is implemented)
 */
#define UDF_IOCTL_MACRO_ENCAP(no_use, func, type) \
	int func##_kernel(udf_kdrv_rep_t, type *);

// TODO: change file name to *UDF_KDRV_IF_DEF_FILE_NAME*
#include "drv_if_cpmk.def"

#undef UDF_IOCTL_MACRO_ENCAP








#ifdef __cplusplus
}
#endif



