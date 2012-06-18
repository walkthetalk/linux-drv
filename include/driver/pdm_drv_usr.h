#pragma once
/*
 * pdm driver header file used by applications in user space
 */

#include <linux/ioctl.h>
#include "drv_cmm_hdr.h"


#ifdef __cplusplus
extern "C" {
#endif

// device file name
#define PDM_DRV_FILE_NAME "cpld0"

// the info app can read after select/poll returned
typedef struct
{
	int slot_id;
	ETinDev_t card_type;
} SDevMngDrvData_t;

typedef struct
{
	ETinDev_t card_type[ETIN_SLOT_MAX];
} SAllCardType_t;

#define PDM_IOC_MAGIC 'P'
enum {
	PDM_IOC_ENABLE_MASTER_INT = _IOW(PDM_IOC_MAGIC, 0, const unsigned int),
	PDM_IOC_ENABLE_STATE_INT = _IOW(PDM_IOC_MAGIC, 1, const unsigned int),
	PDM_IOC_CARD_ON_OFF_LINE = _IOW(PDM_IOC_MAGIC, 2, const SDevMngDrvData_t),
	PDM_IOC_GET_ALL_CARD_TYPE = _IOR(PDM_IOC_MAGIC, 3, SAllCardType_t),
	PDM_IOC_GET_SELF_SLOT_ID = _IOR(PDM_IOC_MAGIC, 4, int),
};

#ifdef __cplusplus
}
#endif


