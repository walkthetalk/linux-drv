#include "prc_mng_info.h"
#include "rt_prc_info.h"
#include "pdm_global.h"
#include "utility/omtlog.h"

// timer to check if some unit need start
#define INTERVAL_CHECK_NEED_START 3000
FD_timer CPrcMngInfo::s_fdTimerStart;
// the list wait to start;
CListHead<CPrcMngInfo> CPrcMngInfo::s_list2Start;

/*
 *
 */
void CPrcMngInfo::start()
{
	// check
	if (getOppositeEnd())
	{
		assert(0);
		// TODO: LOG
		return;
	}

	int ret = subStart();
	if (ret < 0)
	{
		assert(0);
		// TODO: LOG
		return;
	}

	return;
}

/*
 *
 */
void CPrcMngInfo::Term4Stop()
{
	CRtPrcInfo * pChild = getOppositeEnd();
	if (pChild == NULL)
	{
	//		assert(0);
		// TODO: LOG WARNING
		return;
	}
	pChild->signalTerm();
}

/*
 *
 */
void CPrcMngInfo::Kill4Stop()
{
	CRtPrcInfo * pChild = getOppositeEnd();
	if (pChild == NULL)
	{
	//		assert(0);
		// TODO: LOG WARNING
		return;
	}
	pChild->signalKill();
}

/*
 * stop the process related to *this*.
 * NOTE: you need kill the process or some other means before issue the
 * *stop*.
 */
void CPrcMngInfo::stop()
{
	// check
	CRtPrcInfo * pChild = getOppositeEnd();
	if (pChild == NULL)
	{
//		assert(0);
		// TODO: LOG WARNING
		return;
	}

	OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
					"warning: bury child %d\n",pChild->getPid());

	// stop
	int ret = subStop();
	if (ret < 0)
	{
		assert (0);
		// TODO: LOG
		return;
	}

	return;
}

int CPrcMngInfo::sCheckNeedStart(void * pArg)
{

	CPrcMngInfo * pInst = s_list2Start.getFirstEntry();
	if (pInst)
	{
		pInst->start();
	}
	if (s_list2Start.isEmpty())
	{
		CPdmGlobal::s_pPdmPrcBase->StopTimer(s_fdTimerStart);
	}

	return 0;
}

void CPrcMngInfo::sStart(CProcessBase & extPoll)
{
	s_fdTimerStart = extPoll.NewTimer("sCheckNeedStart",
			sCheckNeedStart, INTERVAL_CHECK_NEED_START, NULL);

	return;
}
void CPrcMngInfo::sStop(CProcessBase & extPoll)
{
	extPoll.StopTimer(s_fdTimerStart);
	extPoll.DeleteTimer(s_fdTimerStart);

	return;
}
