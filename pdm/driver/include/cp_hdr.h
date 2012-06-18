#pragma once

#include <linux/kernel.h>

#include "drv_cmm_hdr.h"

#ifdef __cpluscplus
extern "C" {
#endif


#define pdm_pr_fmt(fmt) PDM_DRV_PR_FLAG pr_fmt(fmt) "\n"

#define PDM_DRV_PR_FLAG "pdm_drv "
#define pdm_pr_emerg(fmt, ...) \
        printk(KERN_EMERG pdm_pr_fmt(fmt), ##__VA_ARGS__)
#define pdm_pr_alert(fmt, ...) \
        printk(KERN_ALERT pdm_pr_fmt(fmt), ##__VA_ARGS__)
#define pdm_pr_crit(fmt, ...) \
        printk(KERN_CRIT pdm_pr_fmt(fmt), ##__VA_ARGS__)
#define pdm_pr_err(fmt, ...) \
        printk(KERN_ERR pdm_pr_fmt(fmt), ##__VA_ARGS__)
#define pdm_pr_warning(fmt, ...) \
        printk(KERN_WARNING pdm_pr_fmt(fmt), ##__VA_ARGS__)
#define pdm_pr_warn pdm_pr_warning
#define pdm_pr_notice(fmt, ...) \
        printk(KERN_NOTICE pdm_pr_fmt(fmt), ##__VA_ARGS__)
#define pdm_pr_info(fmt, ...) \
        printk(KERN_INFO pdm_pr_fmt(fmt), ##__VA_ARGS__)
#define pdm_pr_debug(fmt, ...) \
	printk(KERN_DEBUG pdm_pr_fmt(fmt), ##__VA_ARGS__)
#define pdm_pr_cont(fmt, ...) \
        printk(KERN_CONT pdm_prfmt(fmt), ##__VA_ARGS__)
// TODO: add content


#ifdef __cpluscplus
}
#endif


