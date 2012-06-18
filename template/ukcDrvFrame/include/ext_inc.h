/*
 * external header files.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DRV_KERNEL_SIDE
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#else
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/unistd.h>
#endif



/*
 * register type
 */
#define MT_GEN_REG_TYPE(bit_num) volatile u_int##bit_num##_t
#define MT_GEN_REGP_TYPE(bit_num) MT_GEN_REG_TYPE(bit_num) *

#define MT_GEN_VAR_TYPE(bit_num) u_int##bit_num##_t

#ifdef __cplusplus
}
#endif


