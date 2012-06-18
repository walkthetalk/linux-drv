/*
 * upper level user application use this file to control device, (need
 * link with the library generated when compiling driver)
 */
/*
 * NOTE:
 * TODO:
 * please check this file's name, and *UDF_DRV_IF_USR_HDR_FILE_NAME* in
 * your cfg file, they must be same.
 */

#pragma once

#include "drv_op_set.h"

/*
 * include the dss' definition used by your own interfaces
 * TODO: change the file name to *UDF_DRV_IF_DS_HDR_FILE_NAME*
 */
#include "drv_if_ds_cpm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * get the operation set from low level driver
 * TODO: change the function's name
 * NOTE: the name must be *get_drv_op_set_***_fn*, in which the only
 * part you can/need change is *cpm*; change it to your own identificaion,
 * please see macro *UDF_TOKEN_SUBFIX* in your cfg file.
 */
int get_drv_op_set_cpm_fn(udf_drv_op_set_t * rSet);


/*
 * declare all user-defined interfaces used by upper level app.
 * NOTE: all of them has a subfix *_usr*
 */
#define UDF_IOCTL_MACRO_ENCAP(no_use, func, type) \
	int func##_usr(udf_drv_rep_t, type *);
// TODO: change file name to *UDF_DRV_IF_DEF_FILE_NAME*
#include "drv_if_cpm.def"

#undef UDF_IOCTL_MACRO_ENCAP








#ifdef __cplusplus
}
#endif



