#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>

#include <linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");

#include "udf_cfg.h"

#include "drv_cmm_hdr.h"
#include "pdm_drv_ext.h"
#include "udf_dev.h"
#include "udf_misc.h"

#include "udf_dbh_ext.h"

#include "udf_internal_slot_info.h"

static const unsigned int UDF_VAR(g_nMinorDev) = ETIN_SLOT_MAX;
static dev_t UDF_VAR(g_devID) = -1;

static const char UDF_VAR(g_className)[] = "tin_drv_"UDF_CARD_IDF;
static struct class *UDF_VAR(s_pUdfClass) = NULL;

/*
 * store the pointer to card instances.
 */
static void * UDF_VAR(g_udf_dev_set)[ETIN_SLOT_MAX] = { NULL };
static void * UDF_VAR(g_udf_hw_addr)[ETIN_SLOT_MAX] = { NULL };

void * UDF_FUNC(udf_get_slot_hw_base_addr)(int slot)
{
	if (IS_SLOT_ERR(slot))
	{
		ukc_log_err(
			"udf: you want to get hw addr of an invalid slot, slot %d",
			slot);
		return NULL;
	}

	if (UDF_VAR(g_udf_hw_addr)[slot] == NULL)
	{
		ukc_log_err(
			"udf: you want to get hw addr of an card not exist, slot %d",
			slot);
	}

	return UDF_VAR(g_udf_hw_addr)[slot];
}

int UDF_FUNC(udf_online)(int slot, void * card_base_addr)
{
	void * pData = NULL;
	int ret = 0;
	// TODO: if your card type has some constraint, e.g. only can be
	// inserted in some specific slots, you should implement your own
	// logic to checking the number.
	if (IS_SLOT_ERR(slot) || card_base_addr == NULL)
	{
		return -EINVAL;
	}

	if (UDF_VAR(g_udf_hw_addr)[slot] != NULL)
	{
		ukc_log_err(
			"udfonline: the card hw base addr is alreay exist, slot %d",
			slot);
		return -EEXIST;
	}

	// record hw addr
	UDF_VAR(g_udf_hw_addr)[slot] = card_base_addr;

	// init
	pData = UDF_FUNC(udf_main_init)(UDF_VAR(s_pUdfClass),
			MKDEV(MAJOR(UDF_VAR(g_devID)), slot));
	if (pData == NULL)
	{
		UDF_VAR(g_udf_hw_addr)[slot] = NULL;
		return -ENOMEM;
	}

	UDF_VAR(g_udf_dev_set)[slot] = pData;

	ret = UDF_KFUNC(udf_bh_online)(slot);

	return ret;
}

void UDF_FUNC(udf_offline)(int slot)
{
	if (IS_SLOT_ERR(slot))
	{
		return;
	}

	UDF_KFUNC(udf_bh_offline)(slot);

	if (UDF_VAR(g_udf_dev_set)[slot] != NULL)
	{
		UDF_FUNC(udf_main_exit)(UDF_VAR(s_pUdfClass),
			MKDEV(MAJOR(UDF_VAR(g_devID)), slot), UDF_VAR(g_udf_dev_set)[slot]);

		UDF_VAR(g_udf_dev_set)[slot] = NULL;
	}
	else
	{
		ukc_log_err("udf: you want to unload an card not exist, slot %d",
			slot);
	}

	if (UDF_VAR(g_udf_hw_addr)[slot] != NULL)
	{
		UDF_VAR(g_udf_hw_addr)[slot] = NULL;
	}

	return;
}

static int s_init_step = 0;

static void UDF_FUNC(udf_drv_mod_exit)(void)
{
	int i = 0;
	ukc_log_notice("Goodbye, cruel world");

	if (s_init_step >= 3)
	{
		pdm_unreg_online_offline_cb(UDF_CARD_TYPE);
		for (i = 0; i < ARRAY_SIZE(UDF_VAR(g_udf_dev_set)); ++i)
		{
			if (UDF_VAR(g_udf_dev_set)[i] != NULL)
			{
				ukc_log_alert("udf %d not unload, leak!!!", i);
			}
		}
		--s_init_step;
	}

	if (s_init_step >= 2)
	{
		class_destroy(UDF_VAR(s_pUdfClass));
		UDF_VAR(s_pUdfClass) = NULL;
		--s_init_step;
	}

	if (s_init_step >= 1)
	{
		unregister_chrdev_region(UDF_VAR(g_devID), UDF_VAR(g_nMinorDev));
		--s_init_step;
	}
}

static struct drv_vsn_info_t s_drv_vsn_info = {
	.src_vsn = __DRV_SRC_VSN ,
	.udf_vsn = __DRV_UDF_VSN ,
};

static int UDF_FUNC(udf_drv_mod_init)(void)
{
	int i;
	int ret = 0;
	ukc_log_notice("Hello, world\n");
	for (i = 0; i < ARRAY_SIZE(UDF_VAR(g_udf_dev_set)); ++i)
	{
		UDF_VAR(g_udf_dev_set)[i] = NULL;
	}

	// 1. alloc chrdev region
	ret = alloc_chrdev_region(&UDF_VAR(g_devID),
			0, UDF_VAR(g_nMinorDev), UDF_VAR(g_className));
	if (ret < 0)
	{
		ukc_log_err("alloc chrdev fail");
		goto return_ret;
	}
	++s_init_step;

	// 2. create class
	UDF_VAR(s_pUdfClass) = class_create(THIS_MODULE, UDF_VAR(g_className));
	if (IS_OS_PTR_INVALID(UDF_VAR(s_pUdfClass)))
	{
		ukc_log_err("class_create error\n");
		ret = -ENOMEM;
		UDF_VAR(s_pUdfClass) = NULL;
		goto return_ret;
	}
	++s_init_step;

	// 3. reg on/off line callback
	ret = pdm_reg_online_offline_cb(UDF_CARD_TYPE,
			UDF_FUNC(udf_online), UDF_FUNC(udf_offline),&s_drv_vsn_info);
	if (ret < 0)
	{
		goto return_ret;
	}
	++s_init_step;

	ukc_log_notice("INIT success\n");

return_ret:
	if (ret < 0)
	{
		UDF_FUNC(udf_drv_mod_exit)();
	}

	return ret;
}

#define UDF_MODULE_INIT(x) module_init(x)
UDF_MODULE_INIT(UDF_FUNC(udf_drv_mod_init))
#define UDF_MODULE_EXIT(x) module_exit(x)
UDF_MODULE_EXIT(UDF_FUNC(udf_drv_mod_exit))


