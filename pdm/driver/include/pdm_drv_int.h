#pragma once
/*
 * internal use
 */
#include "cp_hdr.h"

#ifdef __cpluscplus
#error "are you ensure using this header file in C++???"
extern "C" {
#endif

int pdm_ctor_mng_info(void);
void pdm_dtor_mng_info(void);

/*
 * here the slot may be not a real slot, indeed, it represents a interrupt.
 */
int pdm_card_online(int slot, ETinDev_t card_type);
void pdm_card_offline(int slot);

/*
 * enable/disable master interrupt
 */
int pdm_enable_master_int(void);
int pdm_disable_master_int(void);

int pdm_handle_slot_int(void);

/*
 * hardware init
 */
int pdm_ctor_hw(void);
void pdm_dtor_hw(void);

/*
 * raw_reg_rw
 */
void pdm_raw_reg_read(char * pBuf, size_t nBytes, loff_t * pOff);
void pdm_raw_reg_write(const char * pBuf, size_t nBytes, loff_t * pOff);


#ifdef __cpluscplus
}
#endif



