#pragma once

#include <linux/ioctl.h>

#include "sifp_drv_cmm.h"

#ifdef __cpluscplus
extern "C" {
#endif




#define TIN_SIFP_IOC_MAGIC 's'
enum
{
	TIN_SIFP_IOC_LED_G_SET	= _IOW(TIN_SIFP_IOC_MAGIC, 0, const ELedOp_t),
	TIN_SIFP_IOC_LED_Y_SET	= _IOW(TIN_SIFP_IOC_MAGIC, 1, const ELedOp_t),
	TIN_SIFP_IOC_LED_R_SET	= _IOW(TIN_SIFP_IOC_MAGIC, 2, const ELedOp_t),
	TIN_SIFP_IOC_INVENTORY_EN = _IOW(TIN_SIFP_IOC_MAGIC, 3, const int),
};







#ifdef __cpluscplus
}
#endif

