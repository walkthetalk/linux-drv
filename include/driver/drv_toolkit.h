#pragma once

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef int (*ioctl_proto_t)(void *, unsigned long);

typedef struct
{
	unsigned int req;
	ioctl_proto_t func;
} ioctl_ele_t;

#define MT_IOCTL_ELE_INIT(op_code, func_name) \
	[_IOC_NR(op_code)] = { .req = (op_code), .func = (ioctl_proto_t)(func_name), }






#ifdef __cplusplus
}
#endif


