#pragma once

#include <linux/types.h>

#ifdef __cpluscplus
#error "are you ensure use this header file in c++??"
extern "C" {
#endif

#include "pdm_pldb611_reg_def.h"
#include "cpld_cmm_reg_def.h"
/*
 * if running on boards, the address is get through ioremap, so can't be const.
 */
extern struct cpld_b611_t * /*const*/ g_pdm_chip;

extern struct cpld_cmm_t * /*const*/ g_cpld_chip[ETIN_SLOT_MAX];




#ifdef __cpluscplus
}
#endif



