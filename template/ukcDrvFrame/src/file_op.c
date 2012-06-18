#include <linux/fs.h>
#include <linux/err.h>
#include <linux/module.h>	// THIS_MODULE

#include <linux/device.h>
#include <asm/uaccess.h>	// copy to user

#include <linux/poll.h>
#include <linux/interrupt.h>	// TASK_INTERRUPTIBLE
#include <linux/sched.h>	// TASK_INTERRUPTIBLE in it in 2.35
#include <linux/mutex.h>

#include <linux/mm.h>		// mmap

#include <linux/compat.h>

#include "pdm_drv_ext.h"
#include "udf_cfg.h"
#include "udf_dev.h"
#include "ukc_mem.h"
#include "udf_misc.h"

#ifdef DRV_USR_MODE
#include "udf_dbh.h"
#define GEN_FUN_NAME(x) UDF_KFUNC(udf_##b##h_##x)
typedef struct UDF_KTYPE(udf_drv_ds) UDF_TYPE(SUDF_DRV_PRV_DATA);
#else
#include "udf_dth.h"
#define GEN_FUN_NAME(x) UDF_FUNC(udf_##t##h_##x)
typedef struct UDF_TYPE(udf_drv_ds) UDF_TYPE(SUDF_DRV_PRV_DATA);
#endif


#define UDF_DRV_OPEN GEN_FUN_NAME(open)
#define UDF_DRV_RELEASE GEN_FUN_NAME(release)
#define UDF_DRV_IOCTL GEN_FUN_NAME(ioctl)
#define UDF_DRV_READ GEN_FUN_NAME(read)
#define UDF_DRV_ISR GEN_FUN_NAME(isr)

#define __UDF_ALLOW_MULTI_ACCESS__

typedef struct
{
	struct cdev m_cdev;
	struct device * m_pDevice;

	wait_queue_head_t	m_inq;	// wait to read something
	volatile unsigned long	m_flags;
#define BIT_CAN_READ 0

#ifdef __UDF_ALLOW_MULTI_ACCESS__
	struct mutex m_mutex;
#endif
	UDF_TYPE(SUDF_DRV_PRV_DATA) * m_pData;
} UDF_TYPE(udf_drv_t);


int UDF_FUNC(udf_open)(struct inode *inode, struct file *filp);
int UDF_FUNC(udf_release)(struct inode *inode, struct file *filp);
ssize_t UDF_FUNC(udf_read)(struct file * pFile, char __user * pBuf, size_t nBytes, loff_t * pOff);
//ssize_t udf_write(struct file *, const char __user *, size_t, loff_t *);
unsigned int UDF_FUNC(udf_poll)(struct file *, struct poll_table_struct *);
long UDF_FUNC(udf_ioctl)(struct file *, unsigned int, unsigned long);
#if HAVE_COMPAT_IOCTL
long UDF_FUNC(udf_compat_ioctl)(struct file * filp, unsigned int cmd, unsigned long arg)
{
#ifdef CONFIG_COMPAT
	return UDF_FUNC(udf_ioctl)(filp, cmd,
			(unsigned long)compat_ptr((compat_uptr_t)arg));
#else
	return -ENOTSUPP;
#endif
}
#endif
int UDF_FUNC(udf_mmap)(struct file *, struct vm_area_struct *);

int UDF_FUNC(udf_isr)(UDF_TYPE(udf_drv_t) *pDrv);

static const struct file_operations UDF_VAR(udf_fops) = {
	.owner = THIS_MODULE,
	.read = UDF_FUNC(udf_read),
	//.write = udf_write,
	.unlocked_ioctl = UDF_FUNC(udf_ioctl),
#if HAVE_COMPAT_IOCTL
	.compat_ioctl = UDF_FUNC(udf_compat_ioctl),
#endif
	.mmap = UDF_FUNC(udf_mmap),
	.open = UDF_FUNC(udf_open),
	.release = UDF_FUNC(udf_release),
	.poll = UDF_FUNC(udf_poll),
};


void * UDF_FUNC(udf_main_init)(struct class * pClass, dev_t devID)
{
	int ret = 0;
	UDF_TYPE(udf_drv_t) * pData;

	pData = UKC_MALLOC(UDF_TYPE(udf_drv_t));
	if (pData == NULL)
	{
		ret = -ENOMEM;
		goto return_ret;
	}

	ukc_log_notice("main_init for dev %d", (int)devID);

	// add dev
	cdev_init (&pData->m_cdev, &UDF_VAR(udf_fops));
	pData->m_cdev.owner = THIS_MODULE;

	ret = cdev_add(&pData->m_cdev, devID , 1);
	if (ret < 0)
	{
		goto return_ret;
	}

	// create device
	pData->m_pDevice = device_create(pClass, NULL, devID, pData,
		UDF_FUNC(getDeviceFilePathRel2Dev)(MINOR(devID)));
	if (IS_OS_PTR_INVALID(pData->m_pDevice))
	{
		ret = -ENOMEM;
		pData->m_pDevice = NULL;
		goto return_ret;
	}

#ifdef __UDF_ALLOW_MULTI_ACCESS__
	mutex_init(&pData->m_mutex);
#endif
	init_waitqueue_head(&pData->m_inq);
	pData->m_flags = 0;
	pData->m_pData = NULL;

return_ret:
	if (ret < 0)
	{
		UDF_FUNC(udf_main_exit)(pClass, devID, pData);
		pData = NULL;
	}
	return pData;
}

void UDF_FUNC(udf_main_exit)(struct class * pClass, dev_t devID, void * arg)
{
	UDF_TYPE(udf_drv_t) * pData = (UDF_TYPE(udf_drv_t) *)arg;
	if (pData == NULL)
	{
		ukc_log_warn("main_exit for dev %d, param err", (int)devID);
		return;
	}

	ukc_log_notice("main_exit for dev %d", (int)devID);

	device_destroy(pClass, devID);

	cdev_del(&pData->m_cdev);

	UKC_FREE(pData);

	return;
}


int UDF_FUNC(udf_open)(struct inode *inode, struct file *filp)
{
	UDF_TYPE(udf_drv_t) * pDrv = container_of(inode->i_cdev, UDF_TYPE(udf_drv_t), m_cdev);
	int slot = MINOR(pDrv->m_pDevice->devt);
	int ret = 0;

	ukc_log_notice("open is called");

	if (filp->private_data != NULL)
	{
		ukc_log_alert("you have private_data already!");
		return -EEXIST;
	}

#ifdef __UDF_ALLOW_MULTI_ACCESS__
	mutex_lock(&pDrv->m_mutex);
#endif
	/*
	 * use minor of device ID to represent slot ID.
	 */
	if (pDrv->m_pData != NULL)
	{
		ukc_log_alert("open: you have malloced for m_pData already!");
		ret = -EEXIST;
		goto return_ret;
	}
	pDrv->m_pData = UDF_DRV_OPEN(slot);
	if (pDrv->m_pData == NULL)
	{
		ukc_log_alert("open drv (th or bh) return NULL!");
		ret = -ENOMEM;
		goto return_ret;
	}

	ret = pdm_request_irq(slot, (pdm_isr_proto_t)UDF_FUNC(udf_isr), pDrv);
	if (ret < 0)
	{
		UDF_DRV_RELEASE(pDrv->m_pData);
		pDrv->m_pData = NULL;
		goto return_ret;
	}

	filp->private_data = pDrv; /* for other methods */
return_ret:
#ifdef __UDF_ALLOW_MULTI_ACCESS__
	mutex_unlock(&pDrv->m_mutex);
#endif

	ukc_log_notice("open result is %d", ret);

	return ret;
}

int UDF_FUNC(udf_release)(struct inode *inode, struct file *pFile)
{
	UDF_TYPE(udf_drv_t) *pDrv = pFile->private_data;
	int slot = MINOR(pDrv->m_pDevice->devt);
	int ret = 0;

#ifdef __UDF_ALLOW_MULTI_ACCESS__
	mutex_lock(&pDrv->m_mutex);
#endif
	pdm_free_irq(slot);
	if (pDrv->m_pData)
	{
		ret = UDF_DRV_RELEASE(pDrv->m_pData);
		pDrv->m_pData = NULL;
	}
#ifdef __UDF_ALLOW_MULTI_ACCESS__
	mutex_unlock(&pDrv->m_mutex);
#endif
	pFile->private_data = NULL;

	return ret;
}

ssize_t UDF_FUNC(udf_read)(struct file * pFile, char __user * pBuf, size_t nBytes, loff_t * pOff)
{
	int ret = 0;
	UDF_TYPE(udf_drv_t) *pDrv = pFile->private_data;


	mutex_lock(&pDrv->m_mutex);

	clear_bit(BIT_CAN_READ, &(pDrv->m_flags));	// must first execute

	ret = UDF_DRV_READ(pDrv->m_pData, pBuf, nBytes);

	mutex_unlock(&pDrv->m_mutex);

	return ret;
}

long UDF_FUNC(udf_ioctl)(struct file *pFile, unsigned int cmd, unsigned long arg)
{
	/*
	 * table for dispatching ioctl
	 */
	typedef int (*UDF_TYPE(ioctl_proto_t))(UDF_TYPE(SUDF_DRV_PRV_DATA) *, unsigned long);
	static const struct
	{
		unsigned int req;
		UDF_TYPE(ioctl_proto_t) func;
		const char * func_name;
	} s_tbl[] =
	{
#ifdef DRV_USR_MODE
#define UDF_IOCTL_MACRO(wr, func, type) \
		{UDF_OPCODE_OF_KIOC_CMD(func), (UDF_TYPE(ioctl_proto_t))func, #func},
#include "udf_dbh_ioctl_if.def"
#else
#define UDF_IOCTL_MACRO(wr, func, type) \
		{UDF_OPCODE_OF_IOC_CMD(func), (UDF_TYPE(ioctl_proto_t))func, #func},
#include "udf_dth_ioctl_if.def"
#endif
	};

	int ret = 0;
	UDF_TYPE(udf_drv_t) *pDrv = pFile->private_data;
	unsigned int idx = _IOC_NR(cmd);

	// lock
#ifdef __UDF_ALLOW_MULTI_ACCESS__
	mutex_lock(&pDrv->m_mutex);
#endif

	// check and issue the corresponding function
	if (ARRAY_SIZE(s_tbl) <= idx
		|| cmd != s_tbl[idx].req)
	{
		ukc_log_err("unknown ioctl cmd idx: %d", idx);
		ret = -ENOIOCTLCMD;
	}
	else
	{
		//UDF_PRINTF("convert ioctl to func: %s", s_tbl[idx].func_name);
		ret = s_tbl[idx].func(pDrv->m_pData, arg);
	}

	// unlock
#ifdef __UDF_ALLOW_MULTI_ACCESS__
	mutex_unlock(&pDrv->m_mutex);
#endif

	return ret;
}

unsigned int UDF_FUNC(udf_poll)(struct file *pFile, struct poll_table_struct *pPollTbl)
{
	UDF_TYPE(udf_drv_t) *pDrv = pFile->private_data;
	unsigned int mask = 0;

	mutex_lock(&pDrv->m_mutex);
	poll_wait(pFile, &pDrv->m_inq, pPollTbl);
	if (test_bit(BIT_CAN_READ, &(pDrv->m_flags)))
		mask |= POLLIN | POLLRDNORM; /* readable */
	mutex_unlock(&pDrv->m_mutex);
	return mask;
}

int UDF_FUNC(udf_mmap)(struct file * pFile, struct vm_area_struct * vma)
{
	UDF_TYPE(udf_drv_t) *pDrv = pFile->private_data;
	int slot = MINOR(pDrv->m_pDevice->devt);

	unsigned long pfn = pdm_get_pfn(slot);
	unsigned long size = pdm_get_addr_space_size(slot);

	pfn += vma->vm_pgoff;
	/*
	 *    |<---------size-------->|
	 *    |<---off--->|<---len--->|
	 * phys          start       end
	 */
	if (((vma->vm_pgoff << PAGE_SHIFT) + vma->vm_end - vma->vm_start)
			> size)
	{
		ukc_log_alert("the size you want is too big");
		return -EFAULT;
	}

	vma->vm_flags |= (VM_IO | VM_RESERVED | VM_SAO);
#ifdef VM_PFN_AT_MMAP
	vma->vm_flags |= VM_PFN_AT_MMAP;
#endif

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	return remap_pfn_range(vma,
			vma->vm_start,
			pfn,
			(vma->vm_end - vma->vm_start),
			vma->vm_page_prot);
}

int UDF_FUNC(udf_isr)(UDF_TYPE(udf_drv_t) *pDrv)
{
	int ret = 0;

	mutex_lock(&pDrv->m_mutex);

	ret = UDF_DRV_ISR(pDrv->m_pData);
	set_bit(BIT_CAN_READ, &(pDrv->m_flags));
	wake_up_interruptible(&pDrv->m_inq);

	mutex_unlock(&pDrv->m_mutex);

	return ret;
}

