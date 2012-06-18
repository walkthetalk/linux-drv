#pragma once

#include "driver/drv_cmm_hdr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * set self ip
 * @return:
 * 	0	success
 * 	<0	fail
 */
int pdm_misc_set_self_ip(void);

#ifdef __cplusplus
}
#endif
