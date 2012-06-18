#pragma once

#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/device.h>

/*
 * copy from *IS_ERR_OR_NULL* in *linux/err.h*
 */
static inline long __must_check IS_OS_PTR_INVALID(const void *ptr)
{
	return IS_ERR(ptr) ? PTR_ERR(ptr) : 0;
}

extern dev_t g_pdmDevID;
extern struct class *g_pPdmDrvClass;

#define MK_PDM_DEV(mi) (MKDEV(MAJOR(g_pdmDevID), mi))

int pdm_drv_main_init(int slot, void * card_base_addr);
void pdm_drv_main_exit(int slot);

int sys_func_drv_main_init(int slot);
void sys_func_drv_main_exit(int slot);

void sys_func_offline_int_notify(u_int32_t int_bmp);

int regs_info_drv_main_init(int slot);
void regs_info_drv_main_exit(int slot);


/*
 * sifp init
 */
int sifp_init_all(struct class * pClass);
void sifp_exit_all(void);

