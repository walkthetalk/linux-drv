/*
 * driver bottom half header file, provide interfaces for low-level
 * read/write register, and so on.
 */

#pragma once

#include "udf_cfg.h"
#include "ext_inc.h"
#include "kdrv_op_set.h"
#include "udf_tbc.h"

#include "udf_ioctl_macro_dbh.h"

#include UDF_KDRV_IF_DS_HDR_FILE_NAME

#ifdef __cplusplus
extern "C" {
#endif


#ifdef DRV_USR_MODE
#ifndef DRV_KERNEL_SIDE
typedef int UDF_KTYPE(udf_bh_rep_t); // usr mode / usr side
#define IS_UDF_BH_REP_ERR(x) ((x) < 0)
#else
#include UDF_KDRV_DS_HDR_FILE_NAME
typedef struct UDF_KTYPE(udf_drv_ds) * UDF_KTYPE(udf_bh_rep_t); // usr mode / kernel side
#define IS_UDF_BH_REP_ERR(x) ((x) == NULL)

int UDF_KFUNC(udf_bh_isr)(UDF_KTYPE(udf_bh_rep_t));	// only UM/KS need
#endif
#else	// DRV_KERNEL_MODE
#ifndef DRV_KERNEL_SIDE
#error "you must define DRV_KERNEL_SIDE for bh when DRV_KERNEL_MODE"
#endif
#include UDF_KDRV_DS_HDR_FILE_NAME
typedef struct UDF_KTYPE(udf_drv_ds) * UDF_KTYPE(udf_bh_rep_t);
#define IS_UDF_BH_REP_ERR(x) ((x) == NULL)
#endif


// open
UDF_KTYPE(udf_bh_rep_t) UDF_KFUNC(udf_bh_open)(int slot);
// release
int UDF_KFUNC(udf_bh_release)(UDF_KTYPE(udf_bh_rep_t));
// read
ssize_t UDF_KFUNC(udf_bh_read)(UDF_KTYPE(udf_bh_rep_t) rep, void * buf, size_t count);
// hw addr
void * UDF_KFUNC(udf_bh_capture_hw_addr)(UDF_KTYPE(udf_bh_rep_t) rep,unsigned int * p_mem_size);
int UDF_KFUNC(udf_bh_release_hw_addr)(void * addr,unsigned int mem_size);

/*
 * auto numbering the ioctl's cmd
 */
#define UDF_IOCTL_MACRO UDF_KIOCTL_MACRO_HDR_AUX
enum
{
#include "udf_dbh_ioctl_if.def"
};

/*
 * the data structure used by the interface
 */
typedef struct
{
	u_int64_t addr;
	volatile u_int64_t val;
} UDF_KTYPE(ioc_arg_rw64_t);

typedef struct
{
	u_int64_t addr;
	volatile u_int32_t val;
} UDF_KTYPE(ioc_arg_rw32_t);

typedef struct
{
	u_int64_t addr;
	volatile u_int16_t val;
} UDF_KTYPE(ioc_arg_rw16_t);

typedef struct
{
	u_int64_t addr;
	volatile u_int8_t val;
} UDF_KTYPE(ioc_arg_rw8_t);


/*
 * generate the cmd's op code of ioctl, and declare the interfaces used
 * by app and driver
 */
#define UDF_IOCTL_MACRO UDF_KIOCTL_MACRO_HDR
#include "udf_dbh_ioctl_if.def"

int UDF_KFUNC(udf_download_fpga)(void *pcpld,const ioc_arg_buf_t *arg);

#ifdef __cplusplus
}
#endif


