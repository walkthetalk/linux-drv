/*
 * this file is for defining interfaces for ioctl.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define _ION(type, nr, size) _IO(type, nr)

#define UDF_OPCODE_OF_KIOC_CMD(func) UDF_KIOC_##func
#define UDF_NBR_OF_KIOC_CMD(func) TIN_KENU_##func

/*
 * declare the prototype of interfaces called by app and ioctl
 */
#define _UDF_KIOC_FUNC_DCL(func, type) \
	int func(UDF_KTYPE(udf_bh_rep_t) arg1, type * arg2)

/*
 * in header files, because the cmd nbr of ioctl is used both by app
 * and driver, so the cmd nbr is defined in header files.
 */
#define UDF_KIOCTL_MACRO_HDR(wr, func, type) \
enum {\
	UDF_OPCODE_OF_KIOC_CMD(func) \
		= _IO##wr(UDF_IOC_MAGIC, UDF_NBR_OF_KIOC_CMD(func), type),\
}; \
_UDF_KIOC_FUNC_DCL(func, type);

/*
 * define the enumeration of cmd, auto numbering.
 */
#define UDF_KIOCTL_MACRO_HDR_AUX(wr, func, type) \
	UDF_NBR_OF_KIOC_CMD(func),

/*
 * the implements in user lib
 */
#define UDF_KIOCTL_MACRO_USR_LIB(wr, func, type) \
int func(int fd, type * pArg) \
{ \
	return ioctl(fd, UDF_OPCODE_OF_KIOC_CMD(func), pArg); \
}



#ifdef __cplusplus
}
#endif

