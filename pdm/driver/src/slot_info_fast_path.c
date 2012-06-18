#include <linux/fs.h>
#include <linux/module.h>	// THIS_MODULE

#include <linux/device.h>
#include <asm/uaccess.h>	// copy to user

#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/compat.h>

#include "drv_toolkit.h"
#include "cp_hdr.h"
#include "pdm_drv_enum.h"
#include "pdm_drv_dev.h"
#include "pdm_drv_int.h"
#include "pdm_drv_sim.h"
#include "sifp_drv_usr.h"
#include "sifp_drv_ext.h"

typedef struct sifp_drv_main
{
	struct cdev	m_cdev;
	struct device	* m_pDevice;

	struct mutex	m_mutex;

	sifp_info_t	slot_info;
} sifp_drv_main_t;
static sifp_drv_main_t s_sifp_drv[ETIN_SLOT_MAX];

static const char * const s_sifp_dev_file_name[ETIN_SLOT_MAX] =
{
	[ETIN_SLOT_SELF] = "tin_sifp_self",
};

static inline int s_is_sifp_slot_err(ETinSlot_t slot)
{
	if (IS_SLOT_ERR(slot)
		|| s_sifp_dev_file_name[slot] == NULL)
	{
		return 1;
	}

	return 0;
}

/*
 * declare function for device file
 */
int sifp_drv_open(struct inode *inode, struct file *filp);
int sifp_drv_release(struct inode *inode, struct file *filp);
ssize_t sifp_drv_read(struct file * pFile, char __user * pBuf, size_t nBytes, loff_t * pOff);
long sifp_drv_ioctl(struct file *, unsigned int, unsigned long);
#if HAVE_COMPAT_IOCTL
long sifp_drv_compat_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
#ifdef CONFIG_COMPAT
	return sifp_drv_ioctl(filp, cmd,
			(unsigned long)compat_ptr((compat_uptr_t)arg));
#else
	return -ENOTSUPP;
#endif
}
#endif


static const struct file_operations sifp_drv_fops = {
	.owner = THIS_MODULE,
	.read = sifp_drv_read,
	.unlocked_ioctl = sifp_drv_ioctl,
#if HAVE_COMPAT_IOCTL
	.compat_ioctl = sifp_drv_compat_ioctl,
#endif
	.open = sifp_drv_open,
	.release = sifp_drv_release,
};

/*
 * slot exit
 */
void sifp_drv_main_exit(ETinSlot_t slot)
{
	sifp_drv_main_t * pData;

	if (s_is_sifp_slot_err(slot))
	{
		pdm_pr_alert("sifp exit param check error, the slot is %s", pdm_slot_2_str(slot));
		return;
	}

	pData = &s_sifp_drv[slot];

	if (pData->m_pDevice)
	{
		device_destroy(pData->m_pDevice->class, pData->m_pDevice->devt);
		pData->m_pDevice = NULL;
	}

	cdev_del(&pData->m_cdev);

	return;
}

/*
 * slot init
 */
int sifp_drv_main_init(ETinSlot_t slot, dev_t devID, struct class * pClass)
{
	int ret = 0;
	sifp_drv_main_t * pData;
	if (s_is_sifp_slot_err(slot))
	{
		pdm_pr_alert("sifp init param check error, the slot is %s", pdm_slot_2_str(slot));
		return -EINVAL;
	}

	pData = &s_sifp_drv[slot];

	// 1. mutex
	mutex_init(&(pData->m_mutex));

	// 2. internal data structure

	// 3. cdev
	cdev_init(&pData->m_cdev, &sifp_drv_fops);
	pData->m_cdev.owner = THIS_MODULE;
	ret = cdev_add(&pData->m_cdev, devID, 1);
	if (ret < 0)
	{
		pdm_pr_alert("sifp Error %d adding char_reg_setup_cdev", ret);
		goto return_ret;
	}

	// 4. create device and file
	pData->m_pDevice = device_create(pClass, NULL, devID,
			pData, s_sifp_dev_file_name[slot]);
	if (IS_OS_PTR_INVALID(pData->m_pDevice))
	{
		pdm_pr_alert("create sifp device %s error",
				s_sifp_dev_file_name[slot]);
		ret = -ENOMEM;
		pData->m_pDevice = NULL;
		goto return_ret;
	}

return_ret:
	if (ret < 0)
	{
		sifp_drv_main_exit(slot);	// unload
	}
	return ret;
}


int sifp_drv_open(struct inode *inode, struct file *filp)
{
	sifp_drv_main_t * pData; /* device information */
	int ret = 0;

	pdm_pr_debug("opening sifp_drv file");

	pData = container_of(inode->i_cdev, sifp_drv_main_t, m_cdev);
	filp->private_data = pData; /* for other methods */

	return ret;
}

int sifp_drv_release(struct inode *inode, struct file *filp)
{
	//sifp_drv_main_t * pData = filp->private_data;

	pdm_pr_debug("releasing sifp_drv file");

	return 0;
}

ssize_t sifp_drv_read(struct file * pFile,
		char __user * pBuf, size_t nBytes, loff_t * pOff)
{
	sifp_drv_main_t *pData = pFile->private_data;
	ssize_t ret = 0;

	mutex_lock(&pData->m_mutex);

	ret = ((pData->slot_info.read_cb)
		? (pData->slot_info.read_cb)(pData->slot_info.read_arg, pBuf, nBytes, pOff)
		: -ENOSYS);

	mutex_unlock(&pData->m_mutex);

	return ret;
}

/*
 * function internal used by ioctl
 * NOTE: indeed, I want implement them as nested functions nested in ioctl,
 * 	but I got this error: initializer element is not computable at load time.
 */
int sifp_ioctl_led_g_set(const sifp_info_t * pInfo, const ELedOp_t * op)
{
	return (pInfo->set_led_green_cb)
		? (pInfo->set_led_green_cb)(pInfo->set_led_arg, *op)
		: -ENOSYS;
}
int sifp_ioctl_led_r_set(const sifp_info_t * pInfo, const ELedOp_t * op)
{
	return (pInfo->set_led_red_cb)
		? (pInfo->set_led_red_cb)(pInfo->set_led_arg, *op)
		: -ENOSYS;
}
int sifp_ioctl_inventory_en(const sifp_info_t * pInfo, const int * op)
{
	return (pInfo->en_inventory_cb)
		? (pInfo->en_inventory_cb)(pInfo->en_inventory_arg, *op)
		: -ENOSYS;
}
int sifp_ioctl_led_y_set(const sifp_info_t * pInfo, const ELedOp_t * op)
{
	return (pInfo->set_led_yellow_cb)
		? (pInfo->set_led_yellow_cb)(pInfo->set_led_arg, *op)
		: -ENOSYS;
}

long sifp_drv_ioctl(struct file *pFile, unsigned int cmd, unsigned long arg)
{
	sifp_drv_main_t * pData = pFile->private_data;
	int ret = 0;

	static const ioctl_ele_t s_ioctl_fun_set[] =
	{
		MT_IOCTL_ELE_INIT(TIN_SIFP_IOC_LED_G_SET, sifp_ioctl_led_g_set),
		MT_IOCTL_ELE_INIT(TIN_SIFP_IOC_LED_Y_SET, sifp_ioctl_led_y_set),
		MT_IOCTL_ELE_INIT(TIN_SIFP_IOC_LED_R_SET, sifp_ioctl_led_r_set),
		MT_IOCTL_ELE_INIT(TIN_SIFP_IOC_INVENTORY_EN, sifp_ioctl_inventory_en),
	};

	unsigned int idx = _IOC_NR(cmd);
	if (ARRAY_SIZE(s_ioctl_fun_set) <= idx
		|| cmd != s_ioctl_fun_set[idx].req)
	{
		// TODO: LOG
		return -ENOIOCTLCMD;
	}

	mutex_lock(&pData->m_mutex);
	ret = (s_ioctl_fun_set[idx].func)(&(pData->slot_info), arg);
	mutex_unlock(&pData->m_mutex);

	return ret;
}

EXPORT_SYMBOL(sifp_reg_for_online);
int sifp_reg_for_online(ETinSlot_t slot, const sifp_info_t * ds)
{
	if (s_is_sifp_slot_err(slot))
	{
		return -EINVAL;
	}

	mutex_lock(&s_sifp_drv[slot].m_mutex);
	s_sifp_drv[slot].slot_info = *ds;
	mutex_unlock(&s_sifp_drv[slot].m_mutex);

	return 0;
}

EXPORT_SYMBOL(sifp_unreg_for_offline);
void sifp_unreg_for_offline(ETinSlot_t slot)
{
	static const sifp_info_t s_null_info;

	if (s_is_sifp_slot_err(slot))
	{
		pdm_pr_alert("sifp unreg param error, vir slot is %d", slot);
		return;
	}

	mutex_lock(&s_sifp_drv[slot].m_mutex);
	s_sifp_drv[slot].slot_info = s_null_info;
	mutex_unlock(&s_sifp_drv[slot].m_mutex);

	return;
}


/*
 * index: minor id enumeration
 * value: slot enumeration
 */
static const ETinSlot_t s_mid_2_slot[] =
{
	[EPDM_DRV_MINOR_ID_SLOT_SELF] = ETIN_SLOT_SELF,
};


int sifp_init_all(struct class * pClass)
{
	int i = 0;
	int ret = 0;

	for (i = 0; i < ARRAY_SIZE(s_mid_2_slot); ++i)
	{
		if (s_mid_2_slot[i] <= 0)
		{
			continue;
		}

		ret = sifp_drv_main_init(s_mid_2_slot[i], MK_PDM_DEV(i), pClass);
		if (ret < 0)
		{
			break;
		}
	}

	if (ret < 0)
	{
		for (--i; i > 0; --i)
		{
			if (s_mid_2_slot[i] <= 0)
			{
				continue;
			}
			sifp_drv_main_exit(s_mid_2_slot[i]);
		}
	}

	return ret;
}

void sifp_exit_all()
{
	int i = ARRAY_SIZE(s_mid_2_slot) - 1;

	for (; i > 0; --i)
	{
		if (s_mid_2_slot[i] <= 0)
		{
			continue;
		}
		sifp_drv_main_exit(s_mid_2_slot[i]);
	}
}
