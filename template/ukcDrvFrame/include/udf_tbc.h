/*
 * the common part will be used by both top half and bottom half
 */

#pragma once

#include "udf_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * for ioctl
 */

#define _UDF_IO(nr) _IO(UDF_IOC_MAGIC,nr)
#define _UDF_IOR(nr, size) _IOR(UDF_IOC_MAGIC,nr,size)
#define _UDF_IOW(nr, size) _IOW(UDF_IOC_MAGIC,nr,size)
#define _UDF_IOWR(nr, size) _IOWR(UDF_IOC_MAGIC,nr,size)

/*
 * for device file name
 */
const char * UDF_FUNC(getDeviceFilePath)(int slot);
int UDF_FUNC(conv_slot_phy_2_vir)(int phy_slot);
const char * UDF_FUNC(getDeviceFilePathRel2Dev)(int slot);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif



#ifdef __cplusplus
}
#endif


