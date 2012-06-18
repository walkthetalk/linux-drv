#pragma once

#include <linux/types.h>
#include "drv_cmm_hdr.h"
#include "sifp_drv_cmm.h"

#ifdef __cpluscplus
#error "only the driver can use this file"
extern "C" {
#endif

/*
 * call-back for read
 * @param:
 *         arg:	the argument recoded when register
 * 	  pBuf:	the buffer provided for filling your string
 * 	nBytes:	the size of the buffer pointed by *pBuf*
 * 	  pOff:	the value it point to represents the offset last reading recorded.
 * 		and you can change it, either increase it or decrease it.
 * @return value:
 * 	if less than 0, it represents an error occurred, or the value is the real
 * 	byte number of data you filled in the buffer.
 */
typedef ssize_t (*sifp_read_cb_t)(void *arg, char * pBuf, size_t nBytes, loff_t * pOff);

typedef int (*sifp_set_led_cb_t)(void * arg, ELedOp_t setting);

typedef int (*sifp_en_inventory_cb_t)(void * arg, int enable);

typedef struct
{
	// call-back for read
	sifp_read_cb_t read_cb;
	void * read_arg;

	// call-back for setting led
	sifp_set_led_cb_t set_led_red_cb;
	sifp_set_led_cb_t set_led_yellow_cb;
	sifp_set_led_cb_t set_led_green_cb;

	// the argument of set_led call-backs
	void * set_led_arg;

	// call-back for enable inventory
	sifp_en_inventory_cb_t en_inventory_cb;
	void * en_inventory_arg;
} sifp_info_t;

/*
 * when card online, it should issue it to register some call-back (with argment),
 * @param:
 * 	slot:	maybe there is no need to interpret it.
 * 	ds:	the caller should fill it.
 */
int sifp_reg_for_online(ETinSlot_t slot, const sifp_info_t * ds);

void sifp_unreg_for_offline(ETinSlot_t slot);

#ifdef __cpluscplus
}
#endif



