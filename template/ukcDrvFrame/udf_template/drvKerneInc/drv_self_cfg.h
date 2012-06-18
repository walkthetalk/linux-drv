#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/*
 * used by:
 * 	device file's name
 * 	class name
 */
#define UDF_CARD_IDF "cpm"

/*
 * pls. see the enumeration *ETinDev_t* in *$repo/src/in/driver/drv_cmm_hdr.h*
 * NOTE:
 *     don't use ETINDEV_PDM
 */
#define UDF_CARD_TYPE ETINDEV_CPM

/* Use 'c' as UDF's magic number */
#define UDF_IOC_MAGIC 'c'

/* the subfix of token */
#define UDF_TOKEN_SUBFIX cpm

#define UDF_DRV_DS_HDR_FILE_NAME "drv_ds_cpm.h"
#define UDF_DRV_IF_DEF_FILE_NAME "drv_if_cpm.def"
#define UDF_DRV_IF_DS_HDR_FILE_NAME "drv_if_ds_cpm.h"
#define UDF_DRV_IF_USR_HDR_FILE_NAME "drv_usr_if_cpm.h"


/* for kernel part */
#define UDF_KDRV_DS_HDR_FILE_NAME "drv_ds_cpmk.h"
#define UDF_KDRV_IF_DEF_FILE_NAME "drv_if_cpmk.def"
#define UDF_KDRV_IF_DS_HDR_FILE_NAME "drv_if_ds_cpmk.h"
#define UDF_KDRV_IF_USR_HDR_FILE_NAME "drv_usr_if_cpmk.h"

#ifdef __cplusplus
}
#endif

