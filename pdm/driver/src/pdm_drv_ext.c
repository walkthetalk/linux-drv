
#include <linux/stddef.h>
#include <linux/module.h>	// export symbol
#include <linux/workqueue.h>
#include <linux/bitops.h>

#include <linux/kernel.h>	// container_of, ARRAY_SIZE
#include <linux/mutex.h>
#include <linux/err.h>


#include "pdm_drv_ext.h"
#include "pdm_drv_int.h"

#include "pdm_drv_regrw.h"
#include "pdm_drv_sim.h"

#define __PDM_ALLOW_RMMOD__


DEFINE_SPINLOCK(s_int_reg_lock);

/*
 * card type info
 */
struct card_type_info_t {
	card_online_cb_t cb_online;
	card_offline_cb_t cb_offline;
};

#ifdef __PDM_ALLOW_RMMOD__
static struct mutex s_card_type_mutex;
#endif
static struct card_type_info_t s_card_type_info[ETINDEV_MAX];
static struct drv_vsn_info_t s_drv_vsn_info[ETINDEV_MAX];

/*
 * slot info
 */
struct slot_info_t;
struct slot_work_t
{
	struct work_struct work_data;
	struct slot_info_t * pSlotInfo;
};

struct slot_info_t
{
	struct mutex	slot_mutex;
	int			slot_idx;	// const
	struct card_type_info_t	*type_info;	// online/offline

	// request/free irq
	pdm_isr_proto_t	isr;
	void 			*arg;

	volatile unsigned long	flags;
#define BIT_WORK_IDX 0		// RW: isr
#define BIT_INT_ENABLED 1	// W: app/offline, R: isr scheduled

	struct slot_work_t	work[2];	// const
};

static struct slot_info_t s_slot_info[ETIN_SLOT_MAX];

int pdm_init_all_card_type_info(void)
{
	int i;
	struct card_type_info_t * pTypeInfo;
	for (i = 0; i < ARRAY_SIZE(s_card_type_info); ++i)
	{
		pTypeInfo = &s_card_type_info[i];
		pTypeInfo->cb_online = NULL;
		pTypeInfo->cb_offline = NULL;
	}

#ifdef __PDM_ALLOW_RMMOD__
	mutex_init(&s_card_type_mutex);
#endif

	return 0;
}

int pdm_init_all_drv_vsn_info(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(s_drv_vsn_info); ++i)
	{
		s_drv_vsn_info[i].src_vsn = NULL;
		s_drv_vsn_info[i].udf_vsn = NULL;
	}
	return 0;
}

struct drv_vsn_info_t *pdm_get_drv_vsn_info(ETinDev_t card_type)
{
	if(card_type >= ETINDEV_MAX)
	{
		return NULL;
	}

	return &s_drv_vsn_info[card_type];
}

/*
 * convert the card tye *ETinDev_t* to string.
 * NOTE: please ensure the order is same as ETinDev_t
 */

static const char * const s_cardName[ETINDEV_MAX] =
{
	[ETINDEV_PDM] = "PDM",

	[ETINDEV_MIO] = "MIO",
};


void pdm_dump_drv_vsn_info(void)
{
	int i=0;
	struct drv_vsn_info_t * p_drv_vsn_info = NULL;
	pdm_pr_notice("=================view driver version==================");
	for(i=0;i<ETINDEV_MAX;i++)
	{
		p_drv_vsn_info = &s_drv_vsn_info[i];
		pdm_pr_notice("\t%s driver version:\n", s_cardName[i]);
		pdm_pr_notice("\t\t\t%s\n", p_drv_vsn_info->src_vsn == NULL ? "unkown" : p_drv_vsn_info->src_vsn);
		pdm_pr_notice("\t\t\t%s\n", p_drv_vsn_info->udf_vsn == NULL ? "unkown" : p_drv_vsn_info->udf_vsn);
	}
}

EXPORT_SYMBOL(pdm_reg_online_offline_cb);
int pdm_reg_online_offline_cb(
	ETinDev_t card_type,
	card_online_cb_t cb_online,
	card_offline_cb_t cb_offline,
	struct drv_vsn_info_t * p_drv_vsn_info)
{
	struct card_type_info_t * pTypeInfo;
	int i;
	int ret = 0;

	// check param
	if (IS_CARD_TYPE_ERR(card_type)
		|| cb_online == NULL
		|| cb_offline == NULL)
	{
		pdm_pr_err("reg online offline for %s type card check param",
				pdm_dev_2_str(card_type));
		return -EINVAL;
	}

	pdm_pr_info("start reg online offline for %s", pdm_dev_2_str(card_type));

#ifdef __PDM_ALLOW_RMMOD__
	mutex_lock(&s_card_type_mutex);
#endif
	pTypeInfo = &(s_card_type_info[card_type]);
	// check if registered already
	if (pTypeInfo->cb_online != NULL
		|| pTypeInfo->cb_offline != NULL)
	{
		pdm_pr_warn("on/off-line for %s is registered already", pdm_dev_2_str(card_type));
		ret = -EEXIST;
		goto exit;
	}

	for (i = 0; i < ARRAY_SIZE(s_slot_info); ++i)
	{
		struct slot_info_t * pSlotInfo = &(s_slot_info[i]);
		mutex_lock(&(pSlotInfo->slot_mutex));
		if (pSlotInfo->type_info == pTypeInfo)
		{
			ret = cb_online(i, g_cpld_chip[i]);
			if (ret < 0)
			{
				pdm_pr_alert("type: %s, slot: %s, fail code: %d",
					pdm_dev_2_str(card_type), pdm_slot_2_str(i), ret);
			}
		}
		mutex_unlock(&(pSlotInfo->slot_mutex));
	}

	// save callback
	s_card_type_info[card_type].cb_offline = cb_offline;
	s_card_type_info[card_type].cb_online = cb_online;

	s_drv_vsn_info[card_type].src_vsn = p_drv_vsn_info->src_vsn;
	s_drv_vsn_info[card_type].udf_vsn = p_drv_vsn_info->udf_vsn;

exit:
#ifdef __PDM_ALLOW_RMMOD__
	mutex_unlock(&s_card_type_mutex);
#endif
	pdm_pr_info("reg on/off-line for %s, ret: %d", pdm_dev_2_str(card_type), ret);

	return ret;
}

EXPORT_SYMBOL(pdm_unreg_online_offline_cb);
void pdm_unreg_online_offline_cb(ETinDev_t card_type)
{
	struct card_type_info_t * pTypeInfo;
	card_offline_cb_t cb_offline;
	int i;

	if (IS_CARD_TYPE_ERR(card_type))
	{
		pdm_pr_err("unreg on/off-line for %s check param", pdm_dev_2_str(card_type));
		return;
	}

	pdm_pr_info("start unreg on/off-line for %s", pdm_dev_2_str(card_type));

	s_drv_vsn_info[card_type].src_vsn = NULL;
	s_drv_vsn_info[card_type].udf_vsn = NULL;

	pTypeInfo = &(s_card_type_info[card_type]);

#ifdef __PDM_ALLOW_RMMOD__
	mutex_lock(&s_card_type_mutex);
#endif

	cb_offline = pTypeInfo->cb_offline;
	pTypeInfo->cb_online = NULL;
	pTypeInfo->cb_offline = NULL;

	for (i = 0; i < ARRAY_SIZE(s_slot_info); ++i)
	{
		struct slot_info_t * pSlotInfo = &(s_slot_info[i]);
		mutex_lock(&(pSlotInfo->slot_mutex));
		if (pSlotInfo->type_info == pTypeInfo)
		{
			cb_offline(i);
		}
		mutex_unlock(&(pSlotInfo->slot_mutex));
	}

#ifdef __PDM_ALLOW_RMMOD__
	mutex_unlock(&s_card_type_mutex);
#endif

	pdm_pr_info("unreg on/off-line for %s done!", pdm_dev_2_str(card_type));

	return;
}

/*
 * declare
 */
static inline void pdm_enable_int_reg(int slot);
static inline void pdm_disable_int_reg(int slot);
void slot_int_handler(struct work_struct *work);

int pdm_init_all_slot_info(void)
{
	int i, j;
	for (i = 0; i < ARRAY_SIZE(s_slot_info); ++i)
	{
		struct slot_info_t * pSlotInfo = &s_slot_info[i];
		mutex_init(&(pSlotInfo->slot_mutex));
		pSlotInfo->slot_idx = i;
		pSlotInfo->type_info = NULL;
		pSlotInfo->isr = NULL;
		pSlotInfo->arg = NULL;
		pSlotInfo->flags = 0;
		for (j = 0; j < ARRAY_SIZE(pSlotInfo->work); ++j)
		{
			INIT_WORK(&(pSlotInfo->work[j].work_data), slot_int_handler);
			pSlotInfo->work[j].pSlotInfo = pSlotInfo;
		}
	}

	return 0;
}

EXPORT_SYMBOL(pdm_request_irq);
int pdm_request_irq(int slot, pdm_isr_proto_t isr, void * arg)
{
	struct slot_info_t * pSlotInfo;
	// check param
	if (IS_SLOT_ERR(slot) || isr == NULL)
	{
		pdm_pr_err("req irq for slot %s check param", pdm_slot_2_str(slot));
		return -EINVAL;
	}

	// check if have card in this slot, and if isr registered already
	pSlotInfo = &s_slot_info[slot];
	if (pSlotInfo->type_info == NULL)
	{
		pdm_pr_err("req irq for slot %s: no card", pdm_slot_2_str(slot));
		return -ENODEV;
	}

	if (pSlotInfo->isr != NULL)
	{
		pdm_pr_err("req irq for slot %s: exist already", pdm_slot_2_str(slot));
		return -EEXIST;
	}

	// save
	pSlotInfo->arg = arg;
	pSlotInfo->isr = isr;

	return 0;
}

EXPORT_SYMBOL(pdm_free_irq);
void pdm_free_irq(int slot)
{
	struct slot_info_t * pSlotInfo;
	// check param
	if (IS_SLOT_ERR(slot))
	{
		pdm_pr_err("free irq for slot %s check param", pdm_slot_2_str(slot));
		return;
	}

	// disable intrrupt first
	pdm_disable_int(slot);

	pSlotInfo = &s_slot_info[slot];
	pSlotInfo->isr = NULL;
	pSlotInfo->arg = NULL;

	return;
}

EXPORT_SYMBOL(pdm_enable_int);
int pdm_enable_int(int slot)
{
	struct slot_info_t * pSlotInfo;

	// check param
	if (IS_SLOT_ERR(slot))
	{
		pdm_pr_err("enable int for slot %s check param", pdm_slot_2_str(slot));
		return -EINVAL;
	}

	pSlotInfo = &s_slot_info[slot];
	set_bit(BIT_INT_ENABLED, &(pSlotInfo->flags));

	pdm_enable_int_reg(slot);

	return 0;
}

EXPORT_SYMBOL(pdm_disable_int);
int pdm_disable_int(int slot)
{
	struct slot_info_t * pSlotInfo;

	// check param
	if (IS_SLOT_ERR(slot))
	{
		pdm_pr_err("disable int for slot %s check param", pdm_slot_2_str(slot));
		return -EINVAL;
	}

	/*
	 * don't need mutex, even we are handling interrupt.
	 * because this bit will only be changed by app, and it will be
	 * discovered by isr next time.
	 */
	pSlotInfo = &s_slot_info[slot];
	clear_bit(BIT_INT_ENABLED, &(pSlotInfo->flags));

	pdm_disable_int_reg(slot);

	return 0;
}

#ifdef MT_SLOT_INT_REG_OP
#error "you have defined MT_SLOT_INT_REG_OP"
#endif
#define MT_SLOT_INT_REG_OP(slot_name, mem_name, bit_name) \
static void pesir_##slot_name(int val) \
{ \
	pdm_pr_notice("pesir "#mem_name" "#bit_name" val is %d", val); \
	MT_CHIP_ASS_BIT(g_pdm_chip, mem_name, bit_name, val); \
}

//MT_SLOT_INT_REG_OP(ETIN_SLOT_PDM, int_plugout_mask, C2_ONLINE_INT_MSK)
MT_SLOT_INT_REG_OP(ETIN_SLOT_SELF, int_mask, INT_MASK)
#undef MT_SLOT_INT_REG_OP

typedef void (*int_reg_op_ft)(int val);
#define MT_INT_REG_OP_ASS(slot_enum) [slot_enum] = pesir_##slot_enum
static int_reg_op_ft const s_int_reg_op_func[ETIN_SLOT_MAX] =
{
//	MT_INT_REG_OP_ASS(ETIN_SLOT_PDM),
	MT_INT_REG_OP_ASS(ETIN_SLOT_SELF),
};

static inline void pdm_enable_int_reg(int slot)
{
	if (s_int_reg_op_func[slot])
	{
		unsigned long flags;
		spin_lock_irqsave(&s_int_reg_lock, flags);
		(s_int_reg_op_func[slot])(0x0);
		spin_unlock_irqrestore(&s_int_reg_lock, flags);
	}
}

static inline void pdm_disable_int_reg(int slot)
{
	if (s_int_reg_op_func[slot])
	{
		unsigned long flags;
		spin_lock_irqsave(&s_int_reg_lock, flags);
		(s_int_reg_op_func[slot])(0x1);
		spin_unlock_irqrestore(&s_int_reg_lock, flags);
	}
}

int pdm_enable_master_int(void)
{
//	MT_CHIP_CLR_BIT(g_pdm_chip, int_all_mask, INT_ALL_MASK);

	return 0;
}

int pdm_disable_master_int(void)
{
//	MT_CHIP_SET_BIT(g_pdm_chip, int_all_mask, INT_ALL_MASK);

	return 0;
}

EXPORT_SYMBOL(pdm_query_online);
int pdm_query_online(int slot)
{
	int ret = 0;

	if(slot != ETIN_SLOT_SELF)
	{
		return -EINVAL;
	}

	ret = 1;	// self remain online

	return ret;
}

/*
 * internal use
 */
int pdm_card_online(int slot, ETinDev_t card_type)
{
	struct slot_info_t * pSlotInfo;
	struct card_type_info_t * pTypeInfo;
	int ret = 0;

	// check param
	if (IS_SLOT_ERR(slot) || IS_CARD_TYPE_ERR(card_type))
	{
		pdm_pr_err("online slot %d type %d: check param", slot, card_type);
		return -EINVAL;
	}

	pdm_pr_notice("online slot %s type %s: start", pdm_slot_2_str(slot),
			pdm_dev_2_str(card_type));

#ifdef __PDM_ALLOW_RMMOD__
	mutex_lock(&s_card_type_mutex);
#endif
	// the driver registered already?
	pTypeInfo = &s_card_type_info[card_type];
	// should be no any card plugged in
	pSlotInfo = &s_slot_info[slot];

	mutex_lock(&(pSlotInfo->slot_mutex));

	// have something already?
	if (pSlotInfo->type_info != NULL)
	{
		ret = -EEXIST;
		goto return_exit;
	}

	if (pTypeInfo->cb_online)
	{
		// execute
		ret = (pTypeInfo->cb_online)(slot, g_cpld_chip[slot]);
		if (ret < 0)
		{
			pdm_pr_err("online slot %s type %s, ret %d",
				pdm_slot_2_str(slot), pdm_dev_2_str(card_type), ret);
			goto return_exit;
		}
	}
	else
	{
		pdm_pr_notice("online slot %s type %s: no callback",
			pdm_slot_2_str(slot), pdm_dev_2_str(card_type));
	}
	// save card type
	pSlotInfo->type_info = pTypeInfo;

return_exit:
	mutex_unlock(&(pSlotInfo->slot_mutex));

#ifdef __PDM_ALLOW_RMMOD__
	mutex_unlock(&s_card_type_mutex);
#endif

	pdm_pr_notice("online slot %s type %s: end",
		pdm_slot_2_str(slot), pdm_dev_2_str(card_type));

	return ret;
}

void pdm_card_offline(int slot)
{
	struct slot_info_t * pSlotInfo;
	struct card_type_info_t * pTypeInfo;

	// check param
	if (IS_SLOT_ERR(slot))
	{
		pdm_pr_err("offline slot %d, check param", slot);
		return;
	}

	pdm_pr_notice("offline slot %s", pdm_slot_2_str(slot));

#ifdef __PDM_ALLOW_RMMOD__
	mutex_lock(&s_card_type_mutex);
#endif
	pSlotInfo = &s_slot_info[slot];

	mutex_lock(&(pSlotInfo->slot_mutex));

	// should be some card plugged in
	if (pSlotInfo->type_info == NULL)
	{
		pdm_pr_err("offline slot %s, no device", pdm_slot_2_str(slot));

		goto return_ret;
	}

	// disable intrrupt, free irq
	pdm_disable_int(slot);
	pdm_free_irq(slot);

	pTypeInfo = pSlotInfo->type_info;
	if (pTypeInfo->cb_offline)
	{
		(pTypeInfo->cb_offline)(slot);
	}
	else
	{
		pdm_pr_notice("offline slot %s: no callback", pdm_slot_2_str(slot));
	}

	// clear type info
	pSlotInfo->type_info = NULL;

return_ret:
	mutex_unlock(&(pSlotInfo->slot_mutex));
#ifdef __PDM_ALLOW_RMMOD__
	mutex_unlock(&s_card_type_mutex);
#endif
	return;
}

/*
 * the isr to handle interrupt from all slot (include pseudo slot)
 */
typedef union
{
	u_int64_t v;
	struct
	{
		typeof(g_pdm_chip->oam_int_n) oam_int_n;
	} bf;
} int_reg_bmp_t;
// the size of *v* must bigger or equal the size of *bf*
typedef char int_reg_bmp_t_constraint[
	sizeof(((int_reg_bmp_t *)0)->v) >= sizeof(int_reg_bmp_t) ? 1 : -1];

/*
 * all slots' interrupt bitmap
 */
static const int_reg_bmp_t const s_int_reg_bmp[ETIN_SLOT_MAX] =
{
#define MT_ASS_INT_BMP_FOR_SLOT(index, mem_name, bit_name) \
	[index] = ((int_reg_bmp_t){ .bf.mem_name = { MT_INIT_BF(bit_name, 0x1), }, })

	MT_ASS_INT_BMP_FOR_SLOT(ETIN_SLOT_SELF, oam_int_n, OAM_INT_n),
#undef MT_ASS_INT_BMP_FOR_SLOT
};

int pdm_handle_slot_int(void)
{
	struct slot_info_t * pSlotInfo;
	int work_idx;
	int slot;
	int ret = 0;

	/*
	 * read register to get all slot's interrupt state
	 */
	const int_reg_bmp_t reg_bmp =
	{
		.bf.oam_int_n = g_pdm_chip->oam_int_n,
	};

	/*
	 * traverse all slot to find who has interrupt
	 */
	for (slot = ETIN_SLOT_MIN; slot < ETIN_SLOT_MAX; ++slot)
	{
		if (reg_bmp.v & s_int_reg_bmp[slot].v)
		{
			pdm_disable_int_reg(slot);
			pSlotInfo = &(s_slot_info[slot]);
			work_idx = (test_and_change_bit(BIT_WORK_IDX, &(pSlotInfo->flags)) == 0 ? 0 : 1);
			schedule_work(&(pSlotInfo->work[work_idx].work_data));
			++ret;
		}
	}

	return ret;
}

/*
 * will run in work-queue
 */
void slot_int_handler(struct work_struct *work)
{
	struct slot_work_t * pSlotWork = container_of(work, struct slot_work_t, work_data);
	struct slot_info_t * pSlotInfo = pSlotWork->pSlotInfo;

	int ret;

	mutex_lock(&(pSlotInfo->slot_mutex));
	if (test_bit(BIT_INT_ENABLED, &(pSlotInfo->flags))	// mutex with offline
		&& pSlotInfo->isr)
	{
		ret = (pSlotInfo->isr)(pSlotWork->pSlotInfo->arg);
		if (ret > 0)
		{
			pdm_enable_int_reg(pSlotInfo->slot_idx);
		}
	}
	mutex_unlock(&(pSlotInfo->slot_mutex));
}

/*
 * initialize
 */
int pdm_ctor_mng_info()
{
	pdm_pr_notice("init mng info");

	pdm_init_all_card_type_info();
	pdm_init_all_drv_vsn_info();
	pdm_init_all_slot_info();

	return 0;
}

void pdm_dtor_mng_info(void)
{
	return;
}


