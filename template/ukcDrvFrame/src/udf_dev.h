
#include <linux/cdev.h>
#include <linux/err.h>

#include "udf_cfg.h"


/*
 * copy from *IS_ERR_OR_NULL* in *linux/err.h*
 */
static inline long __must_check IS_OS_PTR_INVALID(const void *ptr)
{
	return !ptr;// || IS_ERR_VALUE((unsigned long)ptr);
}

void * UDF_FUNC(udf_main_init)(struct class * pClass, dev_t devID);
void UDF_FUNC(udf_main_exit)(struct class * pClass, dev_t devID, void * arg);

