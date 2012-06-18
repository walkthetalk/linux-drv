/*
 * driver top half header file, the app should include it to control
 * the device.
 */
#pragma once

#include "udf_cfg.h"
#include "ukc_misc.h"
#include "ext_inc.h"
#include "udf_tbc.h"

#include "udf_ioctl_macro_dth.h"

#include UDF_DRV_IF_DS_HDR_FILE_NAME

#ifdef __cplusplus
extern "C" {
#endif


#ifdef DRV_USR_MODE
#ifndef DRV_KERNEL_SIDE
#include UDF_DRV_DS_HDR_FILE_NAME
typedef struct UDF_TYPE(udf_drv_ds) * UDF_TYPE(udf_th_rep_t);	// usr mode driver
#define IS_OPEN_ERR(x) (x == NULL)
#else
#error "top half must not *DRV_KERNEL_SIDE* in *DRV_USR_MODE*"
#endif
#else
#ifndef DRV_KERNEL_SIDE
typedef int UDF_TYPE(udf_th_rep_t);		// kernel mode / usr side
#define IS_OPEN_ERR(x) (x < 0)
#else
#include UDF_DRV_DS_HDR_FILE_NAME
typedef struct UDF_TYPE(udf_drv_ds) * UDF_TYPE(udf_th_rep_t);	// kernel mode / kernel side
#define IS_OPEN_ERR(x) (x == NULL)

int UDF_FUNC(udf_th_isr)(UDF_TYPE(udf_th_rep_t));	// only KM/KS need
#endif
#endif

void * UDF_FUNC(udf_th_capture_hw_addr)(int fd,unsigned int * p_mem_size);
int UDF_FUNC(udf_th_release_hw_addr)(void * addr,unsigned int mem_size);

// open
UDF_TYPE(udf_th_rep_t) UDF_FUNC(udf_th_open)(int slot);
// release
int UDF_FUNC(udf_th_release)(UDF_TYPE(udf_th_rep_t) rep);
// read
ssize_t UDF_FUNC(udf_th_read)(
	UDF_TYPE(udf_th_rep_t) rep,
	void * buf,
	size_t count);


/*
 * auto numbering the ioctl's cmd
 */
#define UDF_IOCTL_MACRO UDF_IOCTL_MACRO_HDR_AUX
enum
{
#include "udf_dth_ioctl_if.def"
};

/*
 * generate the cmd's op code of ioctl, and declare the interfaces used
 * by app and driver
 */
#define UDF_IOCTL_MACRO UDF_IOCTL_MACRO_HDR
#include "udf_dth_ioctl_if.def"

/*
 * download fpga
 */
#ifdef DRV_USR_MODE
int UDF_FUNC(udf_download_fpga)(struct UDF_TYPE(udf_drv_ds) * pData,const ioc_arg_buf_t *arg);
#else
int UDF_FUNC(udf_download_fpga)(const ioc_arg_buf_t *arg);
#endif

#ifdef __cplusplus
}
#endif


