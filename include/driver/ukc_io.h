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


#ifndef DRV_KERNEL_SIDE
#include <stdio.h>
#define UKC_PRINTF(fmt, ...) \
        printf(fmt"\n", ##__VA_ARGS__)
/*
 * copy from linux kernel source code
 */
#define	KERN_EMERG	"<0>"	/* system is unusable			*/
#define	KERN_ALERT	"<1>"	/* action must be taken immediately	*/
#define	KERN_CRIT	"<2>"	/* critical conditions			*/
#define	KERN_ERR	"<3>"	/* error conditions			*/
#define	KERN_WARNING	"<4>"	/* warning conditions			*/
#define	KERN_NOTICE	"<5>"	/* normal but significant condition	*/
#define	KERN_INFO	"<6>"	/* informational			*/
#define	KERN_DEBUG	"<7>"	/* debug-level messages			*/

/* Use the default kernel loglevel */
#define KERN_DEFAULT	"<d>"

#include <stddef.h>	// provide offset_of
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})
#else
#include <linux/kernel.h>
#define UKC_PRINTF(fmt, ...) \
        printk(fmt, ##__VA_ARGS__)
#endif

/*
 * the new print level: temporary
 * it should be used with a switch
 */
#define KERN_TMP	"<t>"	/* temporary-level messages			*/

#define ukc_pr_emerg(fmt, ...) \
		UKC_PRINTF(KERN_EMERG UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)
#define ukc_pr_alert(fmt, ...) \
		UKC_PRINTF(KERN_ALERT UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)
#define ukc_pr_crit(fmt, ...) \
		UKC_PRINTF(KERN_CRIT UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)
#define ukc_pr_err(fmt, ...) \
		UKC_PRINTF(KERN_ERR UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)
#define ukc_pr_warning(fmt, ...) \
		UKC_PRINTF(KERN_WARNING UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)
#define ukc_pr_warn ukc_pr_warning
#define ukc_pr_notice(fmt, ...) \
		UKC_PRINTF(KERN_NOTICE UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)
#define ukc_pr_info(fmt, ...) \
		UKC_PRINTF(KERN_INFO UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)
#define ukc_pr_debug(fmt, ...) \
		UKC_PRINTF(KERN_DEBUG UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)
#define ukc_pr_default(fmt, ...) \
		UKC_PRINTF(KERN_DEFAULT UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__)

#define ukc_snprintf(buf, len, fmt, ...) \
		snprintf(buf, len, fmt, ##__VA_ARGS__)

#define ukc_snprintf_line(buf, len, fmt, ...) \
		snprintf(buf, len, fmt"\n", ##__VA_ARGS__)

/*
 * print temporary
 * @argument:
 * 		condition: switch, if true, then print, else nothing to do.
 * 		fmt: print format, used with the *...*
 * why the condition is not a internal variable?
 * answer:	yes, it can, but if so, where it should be stored? if it is
 * 		global, then all this type card will use it when the driver
 * 		running in kernel mode, which is not expected. If we want control
 * 		the specific slot, we must pass an argument to represent it,
 * 		well, then, the better way is pass the condition directly.
 */
#define ukc_pr_tmp(condition, fmt, ...) \
	do { \
		if (condition) {\
			UKC_PRINTF(KERN_TMP UDF_CARD_IDF"_drv: " fmt, ##__VA_ARGS__); \
		} \
	} while (0)

typedef struct
{
	const u_int8_t * buf;
	u_int32_t nBytes;
} ioc_arg_buf_t;

#ifdef __cplusplus
}
#endif


