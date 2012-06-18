#pragma once

/*
 * identify card
 */
#include "drv_self_cfg.h"
#ifndef UDF_CARD_IDF
#error "you should define UDF_CARD_IDF, in your drv_self_cfg.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*
 * identifier
 */
#ifdef DRV_USR_MODE
#define UKC_LOG_MODE u
#else
#define UKC_LOG_MODE k
#endif

#ifndef DRV_KERNEL_SIDE
#define UKC_LOG_SIDE lib
#else
#define UKC_LOG_SIDE ko
#endif
// NOTE:
//    this log id cannot represent the module id like user app, because this
//    driver is used by a class of boards; if you want log for specific slot,
//    please print by yourself (use the arguments of ukc_log_*** serials APIs).
// workround:
//   indeed, we can provide APIs with an argument to represent slot id, but, some
//   logs may be class level, not slot level. so what we provided is a way,
//   how to use it is your business. Good luck!!!
#define MT_STRFICATION(s) MT_STRFICATION_(s)
#define MT_STRFICATION_(s) #s
#define UKC_LOG_ID UDF_CARD_IDF "_" "drv_" MT_STRFICATION(UKC_LOG_MODE) "(" MT_STRFICATION(UKC_LOG_SIDE) ")"

/*
 * the original log api
 */
#ifndef DRV_KERNEL_SIDE
// user side
#include <syslog.h>
#define UKC_LOG(lvl, fmt, ...) \
        syslog(LOG_USER | LOG_##lvl, UKC_LOG_ID " " fmt, ##__VA_ARGS__)
#else
// kernel side
#include <linux/kernel.h>
#define UKC_LOG(lvl, fmt, ...) \
        printk(KERN_##lvl UKC_LOG_ID " " fmt, ##__VA_ARGS__)
#endif


/*
 * log api provided by u(ser)d(river)f(rame).
 * the postfix of the APIs, e.g. emerge of ukc_log_emerge, means the log level.
 */
#define ukc_log_emerg(fmt, ...) \
		UKC_LOG(EMERG, fmt, ##__VA_ARGS__)
#define ukc_log_alert(fmt, ...) \
		UKC_LOG(ALERT, fmt, ##__VA_ARGS__)
#define ukc_log_crit(fmt, ...) \
		UKC_LOG(CRIT, fmt, ##__VA_ARGS__)
#define ukc_log_err(fmt, ...) \
		UKC_LOG(ERR, fmt, ##__VA_ARGS__)
#define ukc_log_warning(fmt, ...) \
		UKC_LOG(WARNING, fmt, ##__VA_ARGS__)
#define ukc_log_warn ukc_pr_warning
#define ukc_log_notice(fmt, ...) \
		UKC_LOG(NOTICE, fmt, ##__VA_ARGS__)
#define ukc_log_info(fmt, ...) \
		UKC_LOG(INFO, fmt, ##__VA_ARGS__)
#define ukc_log_debug(fmt, ...) \
		UKC_LOG(DEBUG, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif


