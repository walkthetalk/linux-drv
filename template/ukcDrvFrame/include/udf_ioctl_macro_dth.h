/*
 * this file is for defining interfaces for ioctl.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define _ION(type, nr, size) _IO(type, nr)

#define UDF_OPCODE_OF_IOC_CMD(func) UDF_IOC_##func
#define UDF_NBR_OF_IOC_CMD(func) TIN_ENU_##func

/*
 * declare the prototype of interfaces called by app and ioctl
 */
#define _UDF_IOC_FUNC_DCL(func, type) \
	int func(UDF_TYPE(udf_th_rep_t) arg1, type * arg2)

/*
 * in header files, because the cmd nbr of ioctl is used both by app
 * and driver, so the cmd nbr is defined in header files.
 */
#define UDF_IOCTL_MACRO_HDR(wr, func, type) \
enum {\
	UDF_OPCODE_OF_IOC_CMD(func) \
		= _IO##wr(UDF_IOC_MAGIC, UDF_NBR_OF_IOC_CMD(func), type),\
};\
_UDF_IOC_FUNC_DCL(func, type);

/*
 * define the enumeration of cmd, auto numbering.
 */
#define UDF_IOCTL_MACRO_HDR_AUX(wr, func, type) \
	UDF_NBR_OF_IOC_CMD(func),

/*
 * the implements in user lib
 */
#define UDF_IOCTL_MACRO_USR_LIB(wr, func, type) \
int func(int fd, type * pArg) \
{ \
	return ioctl(fd, UDF_OPCODE_OF_IOC_CMD(func), pArg); \
}

/*
 * the table in ioctl of driver
 */
#define UDF_IOCTL_MACRO_DRV_TBL(wr, func, type) \
	{UDF_OPCODE_OF_IOC_CMD(func), (UDF_TYPE(ioctl_proto_t))func, #func},



#ifdef __cplusplus
}
#endif

