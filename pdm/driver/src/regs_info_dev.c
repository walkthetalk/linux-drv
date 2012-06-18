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
#include "regs_info_drv_usr.h"

typedef struct regs_info_drv_main
{
	struct cdev		m_cdev;
	struct device		* m_pDevice;

	struct mutex		m_mutex;

	int			m_slot;
	loff_t			m_size;
} regs_info_drv_main_t;
static regs_info_drv_main_t s_regs_info_drv;


/*
 * declare function for device file
 */
int regs_info_drv_open(struct inode *inode, struct file *filp);
int regs_info_drv_release(struct inode *inode, struct file *filp);
loff_t regs_info_drv_llseek(struct file *, loff_t, int);
ssize_t regs_info_drv_read(struct file * pFile, char __user * pBuf, size_t nBytes, loff_t * pOff);
ssize_t regs_info_drv_write(struct file *, const char __user *, size_t, loff_t *);
long regs_info_drv_ioctl(struct file *, unsigned int, unsigned long);
#if HAVE_COMPAT_IOCTL
long regs_info_drv_compat_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
#ifdef CONFIG_COMPAT
	return regs_info_drv_ioctl(filp, cmd,
			(unsigned long)compat_ptr((compat_uptr_t)arg));
#else
	return -ENOTSUPP;
#endif
}
#endif


static const struct file_operations regs_info_drv_fops = {
	.owner = THIS_MODULE,
	.llseek = regs_info_drv_llseek,
	.read = regs_info_drv_read,
	.write = regs_info_drv_write,
	.unlocked_ioctl = regs_info_drv_ioctl,
#if HAVE_COMPAT_IOCTL
	.compat_ioctl = regs_info_drv_compat_ioctl,
#endif
	.open = regs_info_drv_open,
	.release = regs_info_drv_release,
};

void regs_info_drv_main_exit(int slot)
{
	regs_info_drv_main_t * pData;
	dev_t devID = MK_PDM_DEV(slot);
	if (slot != EPDM_DRV_MINOR_ID_REGS_INFO)
	{
		pdm_pr_alert("regs_info devID error, the minorID is %d", slot);
		return;
	}

	pdm_pr_debug("regs_info_drv_main_exit");

	pData = &s_regs_info_drv;

	device_destroy(g_pPdmDrvClass, devID);

	cdev_del(&pData->m_cdev);

	return;
}

int regs_info_drv_main_init(int slot)
{
	int ret = 0;
	regs_info_drv_main_t * pData;
	dev_t devID;
	if (slot != EPDM_DRV_MINOR_ID_REGS_INFO)
	{
		pdm_pr_alert("regs_info devID error, the minorID is %d", slot);
		return -EINVAL;
	}

	devID = MK_PDM_DEV(slot);
	pData = &s_regs_info_drv;

	// 1. mutex
	mutex_init(&(pData->m_mutex));

	// 2. internal data structure
	pData->m_slot = slot;
	pData->m_size = 0x100000000LL;

	// 3. cdev
	cdev_init(&pData->m_cdev, &regs_info_drv_fops);
	pData->m_cdev.owner = THIS_MODULE;

	ret = cdev_add(&pData->m_cdev, devID, 1);
	if (ret < 0)
	{
		pdm_pr_alert("Error %d adding char_reg_setup_cdev", ret);
		goto return_ret;
	}

	// 4. create device and file
	pData->m_pDevice = device_create(g_pPdmDrvClass, NULL, devID,
			pData, TIN_REGS_INFO_DRV_FILE_NAME);
	if (IS_OS_PTR_INVALID(pData->m_pDevice))
	{
		pdm_pr_info("devID is %#x, MAJOR is %#x, MINOR is %#x",
			devID, MAJOR(devID), MINOR(devID));

		pdm_pr_alert("create device %s error",
				TIN_REGS_INFO_DRV_FILE_NAME);
		ret = -ENOMEM;
		pData->m_pDevice = NULL;
		goto return_ret;
	}

return_ret:
	if (ret < 0)
	{
		regs_info_drv_main_exit(slot);	// unload
	}
	return ret;
}


int regs_info_drv_open(struct inode *inode, struct file *filp)
{
	regs_info_drv_main_t * pData; /* device information */
	int ret = 0;

	pdm_pr_debug("opening regs_info_drv file");

	pData = container_of(inode->i_cdev, regs_info_drv_main_t, m_cdev);
	filp->private_data = pData; /* for other methods */

	return ret;
}

int regs_info_drv_release(struct inode *inode, struct file *filp)
{
	//regs_info_drv_main_t * pData = filp->private_data;

	pdm_pr_debug("releasing regs_info_drv file");

	return 0;
}

ssize_t regs_info_drv_read(struct file * pFile,
		char __user * pBuf, size_t nBytes, loff_t * pOff)
{
	regs_info_drv_main_t *pData = pFile->private_data;

	mutex_lock(&pData->m_mutex);

	pdm_raw_reg_read(pBuf, nBytes, pOff);

	mutex_unlock(&pData->m_mutex);

	*pOff += nBytes;

	return nBytes;
}

ssize_t regs_info_drv_write(struct file * pFile, const char __user * pBuf, size_t nBytes, loff_t * pOff)
{
	regs_info_drv_main_t * pData = pFile->private_data;

	mutex_lock(&pData->m_mutex);

	pdm_raw_reg_write(pBuf, nBytes, pOff);

	mutex_unlock(&pData->m_mutex);

	return nBytes;
}

long regs_info_drv_ioctl(struct file *pFile, unsigned int cmd, unsigned long arg)
{
	return -ENOIOCTLCMD;;
}

loff_t regs_info_drv_llseek(struct file *pFile, loff_t off, int whence)
{
	regs_info_drv_main_t *pData = pFile->private_data;
	loff_t newpos = 0;

	pdm_pr_alert("pdm raw reg seek: %lld\t%d", off, whence);
	switch(whence)
	{
		case SEEK_SET:
			newpos = off;
			break;
		case SEEK_CUR:
			if (pData->m_size - pFile->f_pos <= off)
			{
				return -EINVAL;
			}
			newpos = pFile->f_pos + off;
			break;
		case SEEK_END:
			if (off >= 0)
			{
				return -EINVAL;
			}
			newpos = pData->m_size + off;
			break;
	}

	if (newpos < 0 || pData->m_size <= newpos)
	{
		return -EINVAL;
	}

	pFile->f_pos = newpos;
	pdm_pr_alert("pdm raw reg seek new pos: %lld", newpos);
	return newpos;
}
