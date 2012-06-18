#include <linux/fs.h>
#include <linux/module.h>	// THIS_MODULE

#include <linux/device.h>
#include <asm/uaccess.h>	// copy to user
#include <asm/io.h>

#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/compat.h>

#include "cp_hdr.h"
#include "pdm_drv_enum.h"
#include "pdm_drv_dev.h"
#include "pdm_drv_usr.h"
#include "pdm_drv_int.h"
#include "pdm_drv_ext.h"

#include "cpld_cmm_reg_def.h"
#include "pdm_drv_regrw.h"

#include "pdm_drv_sim.h"


typedef struct
{
	struct list_head m_hdr;
	SDevMngDrvData_t m_data;
	struct cpld_cmm_t * m_pCpld;
} cardDataWithHdr_t;

void initCardData(cardDataWithHdr_t * pData, struct list_head * pHead, int slotID)
{
	list_add_tail(&pData->m_hdr, pHead);
	pData->m_data.slot_id = slotID;
	pData->m_data.card_type = ETINDEV_NULL;

	return;
}

typedef struct pdm_drv_main
{
	struct cdev		m_cdev;
	struct device	* m_pDevice;

	struct mutex	m_mutex;

	int			m_slot;

	wait_queue_head_t	m_inq;
	wait_queue_head_t	m_outq;
	struct list_head	m_hdData;
	struct list_head	m_hd2Read;
	cardDataWithHdr_t	m_cardData[ETIN_SLOT_MAX];
} pdm_drv_main_t;
static pdm_drv_main_t s_arr_pdm_drv[1];


int pdm_drv_open(struct inode *inode, struct file *filp);
int pdm_drv_release(struct inode *inode, struct file *filp);
ssize_t pdm_drv_read(struct file * pFile, char __user * pBuf, size_t nBytes, loff_t * pOff);
ssize_t pdm_drv_write(struct file *, const char __user *, size_t, loff_t *);
unsigned int pdm_drv_poll(struct file *, struct poll_table_struct *);
long pdm_drv_ioctl(struct file *, unsigned int, unsigned long);
#if HAVE_COMPAT_IOCTL
long pdm_drv_compat_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
#ifdef CONFIG_COMPAT
	return pdm_drv_ioctl(filp, cmd,
			(unsigned long)compat_ptr((compat_uptr_t)arg));
#else
	return -ENOTSUPP;
#endif
}
#endif

int pdm_ioctl_enable_master_int(pdm_drv_main_t * pData, const unsigned int * isEnable);
int pdm_ioctl_enable_state_int(pdm_drv_main_t * pData, const unsigned int * isEnable);
int pdm_ioctl_card_on_off_line(pdm_drv_main_t * pData, const SDevMngDrvData_t * pArg);
int pdm_ioctl_get_all_card_type(pdm_drv_main_t * pData, SAllCardType_t * pArg);
int pdm_ioctl_get_self_slot_id(pdm_drv_main_t * pData, int * pArg);


static const struct file_operations pdm_drv_fops = {
	.owner = THIS_MODULE,
	.read = pdm_drv_read,
	.write = pdm_drv_write,
	.unlocked_ioctl = pdm_drv_ioctl,
#if HAVE_COMPAT_IOCTL
	.compat_ioctl = pdm_drv_compat_ioctl,
#endif
	.open = pdm_drv_open,
	.release = pdm_drv_release,
	.poll = pdm_drv_poll,
};

int pdm_drv_main_init(int slot, void * card_base_addr)
{
	int ret = 0;
	pdm_drv_main_t * pData;
	dev_t devID;
	int i;
	/*
	 * multiplexing the *slot* as device minor id and pseudo slot no.
	 */
	if (slot != EPDM_DRV_MINOR_ID_PDM_SELF
		|| slot != ETIN_SLOT_PDM
		|| slot < 0 || ARRAY_SIZE(s_arr_pdm_drv) <= slot)
	{
		pdm_pr_alert("devID error, the minorID is %d", slot);
		return -EINVAL;
	}

	devID = MK_PDM_DEV(slot);
	pData = &s_arr_pdm_drv[slot];

	// 1. mutex
	mutex_init(&(pData->m_mutex));

	// 2. internal data structure
	pData->m_slot = slot;
	init_waitqueue_head(&pData->m_inq);
	init_waitqueue_head(&pData->m_outq);
	INIT_LIST_HEAD(&pData->m_hdData);
	INIT_LIST_HEAD(&pData->m_hd2Read);
	// addin default list
	for (i = 0; i < ARRAY_SIZE(pData->m_cardData); ++i)
	{
		initCardData(&pData->m_cardData[i], &pData->m_hdData, (int)i);
	}

	// 3. cdev
	cdev_init (&pData->m_cdev, &pdm_drv_fops);
	pData->m_cdev.owner = THIS_MODULE;

	ret = cdev_add(&pData->m_cdev, devID, 1);
	if (ret < 0)
	{
		pdm_pr_alert("Error %d adding char_reg_setup_cdev", ret);
		goto return_ret;
	}

	// 4. create device and file
	pData->m_pDevice = device_create(g_pPdmDrvClass, NULL, devID,
			pData, PDM_DRV_FILE_NAME);
	if (IS_OS_PTR_INVALID(pData->m_pDevice))
	{
#if 1
		pdm_pr_info("g_pdmDevID is %#x, MAJOR is %#x, MINOR is %#x",
			g_pdmDevID, MAJOR(g_pdmDevID), MINOR(g_pdmDevID));
#endif
		pdm_pr_alert("create device %s error", PDM_DRV_FILE_NAME);
		ret = -ENOMEM;
		pData->m_pDevice = NULL;
		goto return_ret;
	}

return_ret:
	if (ret < 0)
	{
		pdm_drv_main_exit(slot);	// unload
	}
	return ret;
}

void pdm_drv_main_exit(int slot)
{
	pdm_drv_main_t * pData;
	dev_t devID = MK_PDM_DEV(slot);
	if (slot != EPDM_DRV_MINOR_ID_PDM_SELF
			|| slot < 0 || ARRAY_SIZE(s_arr_pdm_drv) <= slot)
	{
		pdm_pr_alert("devID error, the minorID is %d", slot);
		return;
	}

	pdm_pr_debug("pdm_drv_main_exit");

	pData = &s_arr_pdm_drv[slot];

	device_destroy(g_pPdmDrvClass, devID);

	cdev_del(&pData->m_cdev);

	return;
}


int pdm_drv_open(struct inode *inode, struct file *filp)
{
	pdm_drv_main_t * pData; /* device information */
	int ret = 0;
#if 0
	if (filp->private_data != NULL)
	{
		pdm_pr_alert("you have private_data already!");
		return -EEXIST;
	}
#endif
	pdm_pr_notice("opening pdm_drv file");

	pData = container_of(inode->i_cdev, pdm_drv_main_t, m_cdev);
	filp->private_data = pData; /* for other methods */


	return ret;
}

int pdm_drv_release(struct inode *inode, struct file *filp)
{
	//pdm_drv_main_t * pData = filp->private_data;


	pdm_pr_notice("releasing PDM DRV file");

	return 0;
}

ssize_t pdm_drv_read(struct file * pFile, char __user * pBuf, size_t nBytes, loff_t * pOff)
{
	pdm_drv_main_t *pData = pFile->private_data;
	ssize_t ret = 0;
	cardDataWithHdr_t * pPos, *pN;

	mutex_lock(&pData->m_mutex);

	//printk(KERN_NOTICE"I wan't to read something. %lxbytes at %lld", nBytes, *pOff);

	list_for_each_entry_safe(pPos, pN, &pData->m_hd2Read, m_hdr)
	{
		if (sizeof(pPos->m_data) > nBytes)
		{
			break;
		}

		if (copy_to_user(pBuf, &pPos->m_data, sizeof(pPos->m_data)))
		{
			ret = -EFAULT;
			goto exit;
		}

		ret += sizeof(pPos->m_data);
		pBuf += sizeof(pPos->m_data);
		nBytes -= sizeof(pPos->m_data);
		*pOff += sizeof(pPos->m_data);

		list_move_tail(&pPos->m_hdr, &pData->m_hdData);
	}

	if (!list_empty(&pData->m_hd2Read))
	{
		pdm_pr_alert("read incomplete-_-");
	}

exit:
	mutex_unlock(&pData->m_mutex);

	return ret;
}

// forward declare
static void pdm_reset(void);
ssize_t pdm_drv_write(struct file * pFile, const char __user * pBuf, size_t nBytes, loff_t * pOff)
{
	pdm_drv_main_t * pData = pFile->private_data;

#define __PDM_TEST_ON_OFF_LINE__
	ssize_t ret = 0;

	pdm_pr_notice("I want to write %#lx bytes at %lld", (unsigned long)nBytes, *pOff);
#ifndef __PDM_TEST_ON_OFF_LINE__
	if (nBytes % sizeof(((cardDataWithHdr_t *)0)->m_data/*.card_type*/) != 0)
	{
		pdm_pr_alert("write must is x * %d", (int)sizeof(((cardDataWithHdr_t *)0)->m_data));
		return -EINVAL;
	}
#endif

	mutex_lock(&pData->m_mutex);
#ifdef __PDM_TEST_ON_OFF_LINE__
	{
		if (nBytes >= 7 && pBuf[0] == 'e' && '1' <= pBuf[1] && pBuf[1] <= '1')
		{
			int slot = ETIN_SLOT_SELF;
			int type = ((pBuf[3] << 16) | (pBuf[4] << 8) | (pBuf[5]));
			//SDevMngDrvData_t arg;

			switch (type)
			{
			case (('m' << 16) | ('i' << 8) | ('o')):
				type = ETINDEV_MIO;
				break;
			case (('n' << 16) | ('u' << 8) | ('l')):
				type = ETINDEV_NULL;
				break;
			default:
				pdm_pr_alert("I don't know the card type");
				type = ETINDEV_NULL;
				break;
			}

			ret = pdm_sim_slot_on_off_line(slot, type);
			if (ret >= 0)
			{
				pdm_pr_notice("sim slot:%d, type:%d", slot, type);
				ret = nBytes;
			}
			if (pBuf[6] != '\0' && pBuf[6] == ' ' && pBuf[7] == 'r')
			{
				const SDevMngDrvData_t arg = { .slot_id = slot, .card_type = type, };
				pdm_pr_alert("real on-offline slot:%d, type:%d", slot, type);
				ret = pdm_ioctl_card_on_off_line(pData, &arg);
				if (ret >= 0)
				{
					ret = nBytes;
				}
			}
		}
		else
		{
			switch (pBuf[0])
			{
			case 'd':
				{
					SAllCardType_t card_type;
					pdm_ioctl_get_all_card_type(pData, &card_type);
					pdm_dump_base(&card_type);
				}
				break;
			case 'r':
				pdm_reset();
				break;
			case 'v':
				pdm_dump_drv_vsn_info();
				break;
			case 'm':
				{
					switch (pBuf[1])
					{
					case '1':
						pdm_enable_master_int();
						break;
					case '0':
						pdm_disable_master_int();
						break;
					}
				}
				break;
			}

			ret = nBytes;
		}
	}
#else
	if (list_empty(&pData->m_hdData))
	{
		ret = -ENOSPC;
		goto exit;
	}

#define PDM_WRITE_DATA_CONTAINER (pPos->m_data)
	{
	cardDataWithHdr_t * pPos, *pN;
	list_for_each_entry_safe(pPos, pN, &pData->m_hdData, m_hdr)
	{
		if (nBytes < sizeof(PDM_WRITE_DATA_CONTAINER))
		{
			break;
		}
		//pPos->m_data.card_type = *(int *)pBuf;
		if (copy_from_user(&PDM_WRITE_DATA_CONTAINER,
			pBuf, sizeof(PDM_WRITE_DATA_CONTAINER)))
		{
			ret = -EFAULT;
			goto exit;
		}

		pBuf += sizeof(PDM_WRITE_DATA_CONTAINER);
		nBytes -= sizeof(PDM_WRITE_DATA_CONTAINER);

		ret += sizeof(PDM_WRITE_DATA_CONTAINER);

		list_move_tail(&pPos->m_hdr, &pData->m_hd2Read);
	}
	}
#undef PDM_WRITE_DATA_CONTAINER
	if (!list_empty(&pData->m_hd2Read))
	{
		wake_up_interruptible(&pData->m_inq);
	}
#endif
	mutex_unlock(&pData->m_mutex);

	return ret;
}

unsigned int pdm_drv_poll(struct file * pFile, struct poll_table_struct * pPollTbl)
{
	pdm_drv_main_t *pData = pFile->private_data;
	unsigned int mask = 0;

	mutex_lock(&pData->m_mutex);
	poll_wait(pFile, &pData->m_inq, pPollTbl);
	poll_wait(pFile, &pData->m_outq, pPollTbl);
	if (!list_empty(&pData->m_hd2Read))
		mask |= POLLIN | POLLRDNORM; /* readable */
	if (!list_empty(&pData->m_hdData))
		mask |= POLLOUT | POLLWRNORM; /* writable */
	mutex_unlock(&pData->m_mutex);
	return mask;
}

typedef int (*ioctl_proto_t)(pdm_drv_main_t *, unsigned long);

long pdm_drv_ioctl(struct file *pFile, unsigned int cmd, unsigned long arg)
{
	pdm_drv_main_t * pData = pFile->private_data;
	int ret = 0;

	static const struct {
		unsigned int req;
		ioctl_proto_t func;
	} s_ioctl_fun_set[] =
	{
		{ .req = PDM_IOC_ENABLE_MASTER_INT, .func = (ioctl_proto_t)pdm_ioctl_enable_master_int, },
		{ .req = PDM_IOC_ENABLE_STATE_INT, .func = (ioctl_proto_t)pdm_ioctl_enable_state_int, },
		{ .req = PDM_IOC_CARD_ON_OFF_LINE, .func = (ioctl_proto_t)pdm_ioctl_card_on_off_line, },
		{ .req = PDM_IOC_GET_ALL_CARD_TYPE, .func = (ioctl_proto_t)pdm_ioctl_get_all_card_type, },
		{ .req = PDM_IOC_GET_SELF_SLOT_ID, .func = (ioctl_proto_t)pdm_ioctl_get_self_slot_id, }
	};

	unsigned int idx = _IOC_NR(cmd);
	if (ARRAY_SIZE(s_ioctl_fun_set) <= idx
		|| cmd != s_ioctl_fun_set[idx].req)
	{
		pdm_pr_alert("unknown the ioctl cmd: %d, idx: %u", cmd, idx);
		return -ENOIOCTLCMD;
	}

	mutex_lock(&pData->m_mutex);
	ret = (s_ioctl_fun_set[idx].func)(pData, arg);
	mutex_unlock(&pData->m_mutex);

	return ret;
}

/*
 * ioctl function set, don't need lock the mutex of pData.
 */
int pdm_ioctl_enable_master_int(pdm_drv_main_t * pData, const unsigned int * isEnable)
{
/*	int ret = 0;

	if (*isEnable)
	{
		ret = pdm_enable_master_int();
	}
	else
	{
		ret = pdm_disable_master_int();
	}
*/
	return 0;
}

int pdm_ioctl_enable_state_int(pdm_drv_main_t * pData, const unsigned int * isEnable)
{
	int ret = 0;

	return ret;
}

int pdm_ioctl_card_on_off_line(pdm_drv_main_t * pData, const SDevMngDrvData_t * pArg)
{
	if (IS_SLOT_ERR(pArg->slot_id)
		|| (pArg->slot_id == pData->m_slot)
		|| (IS_CARD_N_NULL(pArg->card_type)
			&& IS_CARD_TYPE_ERR(pArg->card_type)))
	{
		return -EINVAL;
	}

	if (pArg->card_type == ETINDEV_NULL)
	{
		pdm_card_offline(pArg->slot_id);
		return 0;
	}
	else
	{
		return pdm_card_online(pArg->slot_id, pArg->card_type);
	}
}

static void pdm_reset(void)
{
//	MT_CHIP_CLR_BIT(g_pdm_chip, cpu_reset, CLR_CPU_RST);
	//MT_CHIP_SET_BIT(g_pdm_chip, cpu_reset, CLR_CPU_RST);

	return;
}

/*
 * check all slot online state and card type
 */
int pdm_ioctl_get_all_card_type(pdm_drv_main_t * pData, SAllCardType_t * pArg)
{
	int i = 0x0;


	for (i = 0; i < ARRAY_SIZE(pData->m_cardData); ++i)
	{
		ETinDev_t card_type;

		card_type = pdm_get_slot_card_type(i, g_cpld_chip[i]);

		pData->m_cardData[i].m_data.card_type = card_type;
		pArg->card_type[i] = card_type;
	}

	return 0;
}

int pdm_ioctl_get_self_slot_id(pdm_drv_main_t * pData, int * pArg)
{
	*pArg = 1;

	return 0;
}
