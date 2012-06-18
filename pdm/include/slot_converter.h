#pragma once

#include "driver/drv_cmm_hdr.h"

#ifdef __cplusplus
extern "C" {
#endif


void gen_slot_converter(int self_phy_slot);

int conv_slot_vir_2_phy(int vir_slot);

int get_selfSlotID(void);

const char * convDig2Str(int i);

const char * getCardTypeStr(ETinDev_t dev);

#ifdef __cplusplus
}
#endif


