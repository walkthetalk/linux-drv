#pragma once

#include "cp_hdr.h"
#include "pdm_drv_usr.h"
#include "cpld_cmm_reg_def.h"

int pdm_sim_slot_on_off_line(ETinSlot_t slot, ETinDev_t type);

const char * pdm_slot_2_str(ETinSlot_t slot);
const char * pdm_dev_2_str(ETinDev_t type);
ETinDev_t pdm_get_slot_card_type(int slot, const struct cpld_cmm_t * pCpld);

void pdm_dump_base(const SAllCardType_t * pCardType);

void pdm_dump_sys_func(void);



