#include <sys/ioctl.h>
#include <stdlib.h>

#include "dynamic_prc_info.h"
#include "rt_prc_info.h"

#include "slot_converter.h"
#include "pdm_global.h"
#include "utility/omtlog.h"

#define INTERVAL_DEV_SPV 1000
#define ONLINE_FILTER_COUNT 3
/*
 * timer for supervising all slots if some card plug-in or plug-out.
 */
FD_timer CDynamicPrcMngInfo::s_fdSpvTimer;//(INTERVAL_DEV_SPV, sCheckDevChange);
/*
 * device fd for reading card type
 */
FD_file CDynamicPrcMngInfo::s_fdCPLD;//(CPLD_DEV_FILE_PATH, sHandleCardChange);

/*
 * it has listed all possible dynamic process.
 * NOTE: please ensure the order is same as ETinSlot_t
 */
#define DYNAMIC_PRC_MNG_ID_BASE 0x200
static CDynamicPrcMngInfo s_dynamicPrcList[ETIN_SLOT_MAX] =
{
#define MT_GEN_DPM_INFO(slot_enum) \
	CDynamicPrcMngInfo(slot_enum, (DYNAMIC_PRC_MNG_ID_BASE + slot_enum))

	MT_GEN_DPM_INFO(ETIN_SLOT_PDM),

	MT_GEN_DPM_INFO(ETIN_SLOT_SELF),
#undef MT_GEN_DPM_INFO
};

const size_t CDynamicPrcMngInfo::s_nMaxDev = COUNTOF(s_dynamicPrcList);

bool CDynamicPrcMngInfo::m_sExitFlag = false;

/*
 * code section
 */
/*
 * constructor
 */
CDynamicPrcMngInfo::CDynamicPrcMngInfo(int slotID, const int mngID) :
	CPrcMngInfo(),
	m_slotID(slotID),
	m_cardType(ETINDEV_NULL),
	m_cardTypeNew(ETINDEV_NULL),
	m_cnt4Filter(0),
	m_mngID(mngID)
{
	// TODO: Add content
}

/*
 * start
 * it will find corresponding card name by id, and then start it.
 */
int CDynamicPrcMngInfo::subStart()
{
	assert(getOppositeEnd() == NULL);

	// 1. check if need start a new process (if has card inserted or not)
	if (!haveCardInserted())
	{
		assert(0);
		// TODO: LOG ERROR
		// del from wait2start list after success.
		this->delFrom2Start();
		return 0;
	}

	// 2. start a new process
	const char * pName = getCardTypeStr(m_cardType);
	if (pName == NULL)
	{
		assert(0);
		// TODO: LOG
		return -1;
	}

	const char * const argv[] =
	{
		TIN_EXE_NAME,
		pName,
		convDig2Str(conv_slot_vir_2_phy(m_slotID)),
		convDig2Str(get_selfSlotID()),
		NULL,
	};

	int ret = CRtPrcInfo::sCapture(*this, TIN_EXE_NAME, argv);
	if (ret < 0)
	{
		assert(0);
		// TODO: LOG ERROR
		return ret;
	}

	// 3. del from wait2start list after success.
	this->delFrom2Start();

	return 0;
}

int CDynamicPrcMngInfo::subStop()
{
	assert(getOppositeEnd());
	// release
	int ret = CRtPrcInfo::sRelease(*getOppositeEnd());
	//if sRelease fail fobid modifying data
	if (ret < 0)
	{
		assert (0);
		// TODO: LOG
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"release rtprcinfo fail, mngID is %d ,ret is %d\n",m_mngID,ret);
		return ret;
	}

	// offline original card
	SDevMngDrvData_t nocardinfo = { m_slotID, ETINDEV_NULL, };
	ret = ioctl(s_fdCPLD, PDM_IOC_CARD_ON_OFF_LINE, &nocardinfo);
	if (ret < 0)
	{
		assert(0);
		// TODO: LOG
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"drv offline fail, mngID is %d ,ret is %d\n",m_mngID,ret);
		return ret;
	}

	// add in list to start
	if (haveCardInserted()&&!CDynamicPrcMngInfo::m_sExitFlag)
	{
		SDevMngDrvData_t newcardinfo = { m_slotID, m_cardType, };
		ret = ioctl(s_fdCPLD, PDM_IOC_CARD_ON_OFF_LINE, &newcardinfo);
		if (ret < 0)
		{
			assert(0);
			OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
					"drv online fail, mngID is %d ,ret is %d\n",m_mngID,ret);

			return ret;
		}
		this->addIn2Start();
	}

	return 0;
}

/*
 * after received new card type, issue this if to handle it.
 */
/*
 * NOTE: after the first time's online(step 3 and 4), it will record new type and cnt,
 * but it's possible a offline succeeded(step 2), and then re-online(4), so the
 * filter failed. To handle this case, we cleared *m_cardTypeNew* in step 1.
 */
void CDynamicPrcMngInfo::handleNewType(ETinDev_t newCardType)
{
#if 0
	std::cout << newCardType << "  " << m_cardType << "  "
			<< m_cardTypeNew << "  " << m_cnt4Filter<< std::endl;
#endif
	// 1. if no change, don't care it.
	if (newCardType == m_cardType)
	{
		m_cardTypeNew = ETINDEV_NULL;
		return;
	}

	// 2. if offline occurred, than exit old process
	if (m_cardType != ETINDEV_NULL)
	{
		m_cardType = ETINDEV_NULL;
		m_cardTypeNew = ETINDEV_NULL;

		CRtPrcInfo * pObj = getOppositeEnd();
		if (pObj)	// alreay started
		{
			// NOTE: maybe it is already killing for live reason.
			assert(pObj->getState() != EPRC_RT_NULL);
			if (pObj->getState() == EPRC_RT_NORMAL)
			{
				CRtPrcInfo::sTerm(*pObj);
			}
		}
		else	// waiting to start
		{
			this->delFrom2Start();
			SDevMngDrvData_t nocardinfo = { m_slotID, ETINDEV_NULL, };
			int ret = ioctl(s_fdCPLD, PDM_IOC_CARD_ON_OFF_LINE, &nocardinfo);
			if (ret < 0) {
				 perror("slot offline ,at the situation process is already killed ");
			}

		}

		return;
	}

	assert(newCardType != ETINDEV_NULL);
	// 3. record new card type (used by filter)
	if (newCardType != m_cardTypeNew)
	{
		m_cardTypeNew = newCardType;
		m_cnt4Filter = 0;
		return;
	}

	// 4. online filter
	++m_cnt4Filter;
	if (m_cnt4Filter >= ONLINE_FILTER_COUNT)
	{
		// NOTEO: if the process is present after filter, then the time for exiting
		// is too long, maybe some error occurred.
		if (getOppositeEnd())
		{
			// TODO: LOG;
			OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
					"warning: how long time your process want for exiting? %d \n",getOppositeEnd()->getPid());

			return;
		}

		m_cnt4Filter = 0;
		m_cardType = newCardType;
		m_cardTypeNew = ETINDEV_NULL;

		// start corresponding process
		SDevMngDrvData_t newcardinfo = { m_slotID, m_cardType, };
		int ret = ioctl(s_fdCPLD, PDM_IOC_CARD_ON_OFF_LINE, &newcardinfo);
		if (ret < 0)
		{
			OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
					"drv offline fail, mngID is %d ,ret is %d\n",m_mngID,ret);

			assert(0);
		}
		this->addIn2Start();
	}

	return;
}

inline int CDynamicPrcMngInfo::getSlotID() const
{
	return m_slotID;
}

bool CDynamicPrcMngInfo::haveCardInserted(void) const
{
	return (m_cardType == ETINDEV_NULL) ? false : true;
}

void CDynamicPrcMngInfo::sStart(CProcessBase & extPoll)
{
	int ret = 0;

	//set exit flag
	m_sExitFlag = false;

	// at start, all slot is NULL.
	/*
	for (int i = 0; i < (int)COUNTOF(s_dynamicPrcList); ++i)
	{
		s_dynamicPrcList[i].addIn2Start();
	}
	*/
	s_fdCPLD = extPoll.RegisterFile<SDevMngDrvData_t>(CPLD_DEV_FILE_PATH,
				(fnCBFile)sHandleCardChange, NULL, ProcessQueueType_high);

	// generate slot converter
	int self_slot = 0;
	ret = ioctl(s_fdCPLD, PDM_IOC_GET_SELF_SLOT_ID, &self_slot);
	if (ret < 0)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"dget self slot id error\n");
		exit(ret);
	}
	gen_slot_converter(self_slot);

	// enable interrupt
	const int arg = 1;
	ret = ioctl(s_fdCPLD, PDM_IOC_ENABLE_STATE_INT, &arg);
	if (ret < 0)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"enable state int error\n");
		exit(ret);
	}
	ret = ioctl(s_fdCPLD, PDM_IOC_ENABLE_MASTER_INT, &arg);
	if (ret < 0)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"enable master int error\n");
		exit(ret);
	}

	// timer
	s_fdSpvTimer = extPoll.NewTimer("sCheckDevChange",sCheckDevChange, INTERVAL_DEV_SPV, NULL);

	return;
}

void CDynamicPrcMngInfo::sTerm4Stop()
{
	for (int i = (int)COUNTOF(s_dynamicPrcList) - 1; i >= 0; --i)
	{
		s_dynamicPrcList[i].Term4Stop();
	}

	return;
}

void CDynamicPrcMngInfo::sKill4Stop()
{
	for (int i = (int)COUNTOF(s_dynamicPrcList) - 1; i >= 0; --i)
	{
		s_dynamicPrcList[i].Kill4Stop();
	}

	return;
}

void CDynamicPrcMngInfo::sStop(CProcessBase & extPoll)
{
	int ret = 0;

	//set exit flag
	m_sExitFlag = true;

	// timer
	extPoll.StopTimer(s_fdSpvTimer);
	extPoll.DeleteTimer(s_fdSpvTimer);

	// disable interrupt
	const int arg = 0;
	ret = ioctl(s_fdCPLD, PDM_IOC_ENABLE_STATE_INT, &arg);
	if (ret < 0)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"disable state int error\n");
		exit(ret);
	}
	ret = ioctl(s_fdCPLD, PDM_IOC_ENABLE_MASTER_INT, &arg);
	if (ret < 0)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
					"disable master int error\n");
		exit(ret);
	}

	for (int i = (int)COUNTOF(s_dynamicPrcList) - 1; i >= 0; --i)
	{
		s_dynamicPrcList[i].stop();
	}

	// unreg fd
	extPoll.UnRegisterFile(s_fdCPLD);

	return;
}

static const u_int32_t s_slot_care_bmp =
		(1 << ETIN_SLOT_SELF);
static inline int dont_care_slot(int i)
{
	if ((1 << i) & s_slot_care_bmp)
	{
		return 0;
	}

	return 1;
}

int CDynamicPrcMngInfo::sCheckDevChange(void * pArg)
{
	// check slot's new state
	ETinDev_t cardtype[ETIN_SLOT_MAX];
	int ret = ioctl(s_fdCPLD, PDM_IOC_GET_ALL_CARD_TYPE, (unsigned long)cardtype);
	if (ret < 0)
	{
		// TODO: LOG
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
					"ioCtl error\n");
		return ret;
	}

	for (size_t i = 0; i < COUNTOF(cardtype); ++i)
	{
		if (dont_care_slot(i))
		{
			continue;
		}

		if (IS_CARD_TYPE_INVALID(cardtype[i]))
		{
			// TODO: LOG ERROR
			assert(0);
			continue;
		}

		CDynamicPrcMngInfo & inst = s_dynamicPrcList[i];
		assert((size_t)(inst.getSlotID()) == i);
		inst.handleNewType(cardtype[i]);
	}

	return 0;
}

/*
 * handle card change from driver
 * @pNewContent:	the new change information
 * @pArg:		no use currently
 */
int CDynamicPrcMngInfo::sHandleCardChange(SDevMngDrvData_t * pNewContent, void * pArg)
{
	OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"handle card %d is %d \n",pNewContent->slot_id,pNewContent->card_type);
	if (IS_SLOT_ERR(pNewContent->slot_id)
		|| IS_CARD_TYPE_INVALID(pNewContent->card_type))
	{
		// TODO: LOG ERROR
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
							"handle card change error\n");
		return -1;
	}

	if (dont_care_slot(pNewContent->slot_id))
	{
		// TODO: LOG
		return 0;
	}

	s_dynamicPrcList[pNewContent->slot_id].handleNewType(
		(ETinDev_t)(pNewContent->card_type));

	return 0;
}


