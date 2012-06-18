#pragma once

#include <linux/types.h>
#include "drv_cmm_hdr.h"

/*
 * NOTE:
 * 	for all *slot* appeared in this file, it's value must be one of the
 * 	enumeration *ETinSlot_t*.
 */
#ifdef __cpluscplus
#error "only the driver can use this file"
extern "C" {
#endif

/*
 * driver version
 */
struct drv_vsn_info_t {
	const char * src_vsn;
	const char * udf_vsn;
};
struct drv_vsn_info_t *pdm_get_drv_vsn_info(ETinDev_t card_type);
void pdm_dump_drv_vsn_info(void);

/*
 * return value:
 * 	if *< 0*, it means *fail*.
 */
typedef int (*card_online_cb_t)(int slot, void * card_base_addr);
typedef void (*card_offline_cb_t)(int slot);
/*
 * reg online/offline callback for specific card type
 * used by other kernel modules' init/exit (e.g. insmod/modprobe,
 * because we will exe all this cmd in the same shell, so no
 * concurrence, and not consider any mutual exclude in it's impletment.)
 *
 * NOTE: the *cb_online* need to do clear-work by itself when *fail*.
 * and when it *fail*, the return value must less than *0*, then pdm
 * module can discover the *fail*.
 */
int pdm_reg_online_offline_cb(
	ETinDev_t card_type,
	card_online_cb_t cb_online,
	card_offline_cb_t cb_offline,
	struct drv_vsn_info_t * p_drv_vsn_info);
void pdm_unreg_online_offline_cb(ETinDev_t card_type);


/*
 * isr prototype, to be defined
 *
 * NOTE: if the return value greater than *0*, the interrupt will be
 * valid next time. But if not (ret <= 0), it's your responsible to
 * enable the interrupt again.
 */
typedef int (*pdm_isr_proto_t)(void * arg);
/*
 * request/free irq for specific slot
 * @isr:	the callback will be issued when corresponding interrupt occurred.
 * @arg:	the argument will passed to *isr* when issued.
 * return value:
 * 	if success, it will *>= 0*, or *< 0*.
 */
int pdm_request_irq(int slot, pdm_isr_proto_t isr, void * arg);
void pdm_free_irq(int slot);

/*
 * enable interrupt for specific slot
 */
int pdm_enable_int(int slot);

/*
 * disable interrupt for specific slot
 */
int pdm_disable_int(int slot);

/*
 *query online state for specific slot
 */
int pdm_query_online(int slot);

/*
 * get the physical address of specific slot
 */
unsigned long pdm_get_pfn(int slot);
unsigned long pdm_get_addr_space_size(int slot);

/*
 * test queue work
 */
int pdm_queue_work_test(void);

#ifdef __cpluscplus
}
#endif



