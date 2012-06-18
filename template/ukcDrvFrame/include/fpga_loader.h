#pragma once

#include "udf_cfg.h"

#include "cpld_cmm_reg_def.h"

#ifdef __cplusplus
extern "C" {
#endif


int UDF_KFUNC(udf_download_fpga_imp)(
	struct cpld_cmm_t * pCpld,
	const u_int8_t * pBuf,
	u_int32_t nBytes);


#ifdef __cplusplus
}
#endif
