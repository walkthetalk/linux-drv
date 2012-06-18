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
#include "sys_func_drv_usr.h"

#include "pdm_drv_regrw.h"

#include "pdm_drv_sim.h"

typedef struct sys_func_drv_main
{
	struct cdev		m_cdev;
	struct device		* m_pDevice;

	struct mutex		m_mutex;

	int			m_slot;

	u_int32_t		hrm_offline_int;

	wait_queue_head_t	m_inq;
	wait_queue_head_t	m_outq;
} sys_func_drv_main_t;
static sys_func_drv_main_t s_sys_func_drv;


/*
 * declare function for device file
 */
int sys_func_drv_open(struct inode *inode, struct file *filp);
int sys_func_drv_release(struct inode *inode, struct file *filp);
ssize_t sys_func_drv_read(struct file * pFile, char __user * pBuf, size_t nBytes, loff_t * pOff);
ssize_t sys_func_drv_write(struct file *, const char __user *, size_t, loff_t *);
unsigned int sys_func_drv_poll(struct file *, struct poll_table_struct *);
long sys_func_drv_ioctl(struct file *, unsigned int, unsigned long);
#if HAVE_COMPAT_IOCTL
long sys_func_drv_compat_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
#ifdef CONFIG_COMPAT
	return sys_func_drv_ioctl(filp, cmd,
			(unsigned long)compat_ptr((compat_uptr_t)arg));
#else
	return -ENOTSUPP;
#endif
}
#endif

/*
 * declare the interfaces for ioctl
 */
static int sys_func_ioctl_hrm_get(sys_func_drv_main_t * pData, u_int32_t * arg);
static int sys_func_ioctl_hrm_tmp_set(sys_func_drv_main_t * pData, const u_int32_t * arg);
static int sys_func_ioctl_hrm_tmp_get(sys_func_drv_main_t * pData, u_int32_t * arg);
static int sys_func_ioctl_blank_get(sys_func_drv_main_t * pData, u_int32_t * arg);
static int sys_func_ioctl_blank_set(sys_func_drv_main_t * pData, const u_int32_t * arg);
static int sys_func_ioctl_opp_online_get(sys_func_drv_main_t * pData, u_int32_t * arg);
static int sys_func_ioctl_iic_sel(sys_func_drv_main_t * pData, iic_sel_arg_t * arg);


static const struct file_operations sys_func_drv_fops = {
	.owner = THIS_MODULE,
	.read = sys_func_drv_read,
	.write = sys_func_drv_write,
	.unlocked_ioctl = sys_func_drv_ioctl,
#if HAVE_COMPAT_IOCTL
	.compat_ioctl = sys_func_drv_compat_ioctl,
#endif
	.open = sys_func_drv_open,
	.release = sys_func_drv_release,
	.poll = sys_func_drv_poll,
};

int sys_func_drv_main_init(int slot)
{
	int ret = 0;
	sys_func_drv_main_t * pData;
	dev_t devID;
	if (slot != EPDM_DRV_MINOR_ID_SYS_FUNC)
	{
		pdm_pr_alert("sys_func devID error, the minorID is %d", slot);
		return -EINVAL;
	}

	devID = MK_PDM_DEV(slot);
	pData = &s_sys_func_drv;

	// 1. mutex
	mutex_init(&(pData->m_mutex));

	// 2. internal data structure
	pData->m_slot = slot;
	init_waitqueue_head(&pData->m_inq);
	init_waitqueue_head(&pData->m_outq);
	pData->hrm_offline_int = 0x0;

	// 3. cdev
	cdev_init(&pData->m_cdev, &sys_func_drv_fops);
	pData->m_cdev.owner = THIS_MODULE;

	ret = cdev_add(&pData->m_cdev, devID, 1);
	if (ret < 0)
	{
		pdm_pr_alert("Error %d adding char_reg_setup_cdev", ret);
		goto return_ret;
	}

	// 4. create device and file
	pData->m_pDevice = device_create(g_pPdmDrvClass, NULL, devID,
			pData, TIN_SYS_FUNC_DRV_FILE_NAME);
	if (IS_OS_PTR_INVALID(pData->m_pDevice))
	{
#if 1
		pdm_pr_info("devID is %#x, MAJOR is %#x, MINOR is %#x",
			devID, MAJOR(devID), MINOR(devID));
#endif
		pdm_pr_alert("create device %s error",
				TIN_SYS_FUNC_DRV_FILE_NAME);
		ret = -ENOMEM;
		pData->m_pDevice = NULL;
		goto return_ret;
	}

return_ret:
	if (ret < 0)
	{
		sys_func_drv_main_exit(slot);	// unload
	}
	return ret;
}

void sys_func_drv_main_exit(int slot)
{
	sys_func_drv_main_t * pData;
	dev_t devID = MK_PDM_DEV(slot);
	if (slot != EPDM_DRV_MINOR_ID_SYS_FUNC)
	{
		pdm_pr_alert("sys_func devID error, the minorID is %d", slot);
		return;
	}

	pdm_pr_debug("sys_func_drv_main_exit");

	pData = &s_sys_func_drv;

	device_destroy(g_pPdmDrvClass, devID);

	cdev_del(&pData->m_cdev);

	return;
}


int sys_func_drv_open(struct inode *inode, struct file *filp)
{
	sys_func_drv_main_t * pData; /* device information */
	int ret = 0;
#if 0
	if (filp->private_data != NULL)
	{
		pdm_pr_alert("you have private_data already!");
		return -EEXIST;
	}
#endif
	pdm_pr_debug("opening sys_func_drv file");

	pData = container_of(inode->i_cdev, sys_func_drv_main_t, m_cdev);
	filp->private_data = pData; /* for other methods */

#if 0
	/* now trim to 0 the length of the device if open was write-only */
	if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
	{
		scull_trim(dev); /* ignore errors */
	}
#endif

	return ret;
}

int sys_func_drv_release(struct inode *inode, struct file *filp)
{
	//sys_func_drv_main_t * pData = filp->private_data;

	pdm_pr_debug("releasing sys_func_drv file");

	return 0;
}

ssize_t sys_func_drv_read(struct file * pFile,
		char __user * pBuf, size_t nBytes, loff_t * pOff)
{
	sys_func_drv_main_t *pData = pFile->private_data;
	ssize_t ret = 0;
	if (nBytes < sizeof(pData->hrm_offline_int))
	{
		return -EINVAL;
	}

	mutex_lock(&pData->m_mutex);

	//printk(KERN_NOTICE"read hrm int. %lxbytes at %lld", nBytes, *pOff);

	*(typeof(pData->hrm_offline_int) *)(pBuf) = pData->hrm_offline_int;
	pData->hrm_offline_int = 0x0;

	mutex_unlock(&pData->m_mutex);

	ret = sizeof(pData->hrm_offline_int);

	return ret;
}

ssize_t sys_func_drv_write(struct file * pFile, const char __user * pBuf, size_t nBytes, loff_t * pOff)
{
	sys_func_drv_main_t * pData = pFile->private_data;

	ssize_t ret = 0;
	int arg = 0;

	//pdm_pr_debug("write sys_func_drv: %s", pBuf);

	if (pBuf[0] != 'd')
	{
		arg = simple_strtoul(&pBuf[1], NULL, 0);
	}

	switch (pBuf[0])
	{
	case 'i':
		sys_func_offline_int_notify(arg);
		break;
	case 'h':
		sys_func_ioctl_hrm_tmp_set(pData, &arg);
		break;
	case 'b':
		sys_func_ioctl_blank_set(pData, &arg);
		break;
	case 'd':
		pdm_dump_sys_func();
		break;
	}

	ret = nBytes;

	return ret;
}

unsigned int sys_func_drv_poll(struct file * pFile, struct poll_table_struct * pPollTbl)
{
	sys_func_drv_main_t *pData = pFile->private_data;
	unsigned int mask = 0;

	mutex_lock(&pData->m_mutex);
	poll_wait(pFile, &pData->m_inq, pPollTbl);
	poll_wait(pFile, &pData->m_outq, pPollTbl);
	if (pData->hrm_offline_int)
		mask |= POLLIN | POLLRDNORM; /* readable */
	mutex_unlock(&pData->m_mutex);
	return mask;
}


long sys_func_drv_ioctl(struct file *pFile, unsigned int cmd, unsigned long arg)
{
	sys_func_drv_main_t * pData = pFile->private_data;
	int ret = 0;

	static const ioctl_ele_t s_ioctl_fun_set[] =
	{
		MT_IOCTL_ELE_INIT(TIN_SYS_FUNC_IOC_HRM_GET, sys_func_ioctl_hrm_get),
		MT_IOCTL_ELE_INIT(TIN_SYS_FUNC_IOC_HRM_TMP_SET, sys_func_ioctl_hrm_tmp_set),
		MT_IOCTL_ELE_INIT(TIN_SYS_FUNC_IOC_HRM_TMP_GET, sys_func_ioctl_hrm_tmp_get),
		MT_IOCTL_ELE_INIT(TIN_SYS_FUNC_IOC_BLANK_GET, sys_func_ioctl_blank_get),
		MT_IOCTL_ELE_INIT(TIN_SYS_FUNC_IOC_BLANK_SET, sys_func_ioctl_blank_set),
		MT_IOCTL_ELE_INIT(TIN_SYS_FUNC_IOC_OPP_ONLINE_GET, sys_func_ioctl_opp_online_get),
		MT_IOCTL_ELE_INIT(TIN_SYS_FUNC_IOC_IIC_SEL, sys_func_ioctl_iic_sel),
	};

	unsigned int idx = _IOC_NR(cmd);
	if (ARRAY_SIZE(s_ioctl_fun_set) <= idx
		|| cmd != s_ioctl_fun_set[idx].req)
	{
		// TODO: LOG
		return -ENOIOCTLCMD;
	}

	mutex_lock(&pData->m_mutex);
	ret = (s_ioctl_fun_set[idx].func)(pData, arg);
	mutex_unlock(&pData->m_mutex);

	return ret;
}

void sys_func_offline_int_notify(u_int32_t int_bmp)
{
	sys_func_drv_main_t * pData = &s_sys_func_drv;

	mutex_lock(&pData->m_mutex);

	pData->hrm_offline_int |= int_bmp;

	/*
	 * wake up app to read the new info
	 */
	if (pData->hrm_offline_int)
	{
		wake_up_interruptible(&pData->m_inq);
	}

	mutex_unlock(&pData->m_mutex);

	return;
}

static int sys_func_ioctl_hrm_get(sys_func_drv_main_t * pData, u_int32_t * arg)
{
//	typeof(g_pdm_chip->ms_state) ms_state = g_pdm_chip->ms_state;
//	*arg = MT_REG_GET_BF(ms_state, CPM1_STATS) | (MT_REG_GET_BF(ms_state, CPM2_STATS) << 1);
	*arg = 0 | (1 << 1);//self master opposite slave

	return 0;
}

static int sys_func_ioctl_hrm_tmp_set(sys_func_drv_main_t * pData, const u_int32_t * arg)
{
//	MT_CHIP_ASS_BF(g_pdm_chip, ms_op, Cpm1_StaTmp, (*arg));
	return 0;
}

static int sys_func_ioctl_hrm_tmp_get(sys_func_drv_main_t * pData, u_int32_t * arg)
{
/*	typeof(g_pdm_chip->ms_op) ms_op = g_pdm_chip->ms_op;

	*arg = (MT_REG_GET_BF(ms_op, Cpm1_StaTmp)
		| (MT_REG_GET_BF(ms_op, Cpm2_StaTmp) << 2));
*/
	return 0;
}

static int sys_func_ioctl_blank_get(sys_func_drv_main_t * pData, u_int32_t * arg)
{
	*arg = MT_CHIP_GET_BF(g_pdm_chip, blank_reg, BLANK_REG);

	return 0;
}

static int sys_func_ioctl_blank_set(sys_func_drv_main_t * pData, const u_int32_t * arg)
{
	MT_CHIP_ASS_BF(g_pdm_chip, blank_reg, BLANK_REG, *arg);

	return 0;
}

static int sys_func_ioctl_opp_online_get(sys_func_drv_main_t * pData, u_int32_t * arg)
{
//	*arg = MT_CHIP_GET_BIT(g_pdm_chip, online_aux, ONLINE_OPP);
	return 0;
}

static int sys_func_ioctl_iic_sel(sys_func_drv_main_t * pData, iic_sel_arg_t * arg)
{
	MT_CHIP_ASS_BF(g_pdm_chip, iic, IIC_DIS, (arg->sel));

	switch (arg->sel)
	{
	case 1:
	{
		break;
	}
	case 2:
	{
		break;
	}
	case 3:
	{
		break;
	}
	default:
		break;
	}

	return 0;
}

