/*
 * Ni Qingliang
 */
#include "static_prc_info.h"
#include "rt_prc_info.h"

#include "slot_converter.h"


#define STATIC_PRC_MNG_ID_BASE 0x100

static CStaticPrcMngInfo s_staticPrcList[MAX_STATIC_PRC_NUM] =
{
	CStaticPrcMngInfo("DE", STATIC_PRC_MNG_ID_BASE + 0x0),
	CStaticPrcMngInfo("APM", STATIC_PRC_MNG_ID_BASE + 0x1),
	CStaticPrcMngInfo("SYS", STATIC_PRC_MNG_ID_BASE + 0x2),
	CStaticPrcMngInfo("AGENT", STATIC_PRC_MNG_ID_BASE + 0x3),
	CStaticPrcMngInfo("UMI", STATIC_PRC_MNG_ID_BASE + 0x4),
};

const size_t CStaticPrcMngInfo::s_nMaxStaticPrc = COUNTOF(s_staticPrcList);

int CStaticPrcMngInfo::subStart()
{
	//std::cout << "start " << m_mngID << std::endl;
	assert(getOppositeEnd() == NULL);


	const char * const argv[] =
	{
		TIN_EXE_NAME,
		m_pBinName,
		convDig2Str(get_selfSlotID()),
		NULL,
	};
	int ret = CRtPrcInfo::sCapture(*this, TIN_EXE_NAME, argv);

	if (ret < 0)
	{
		assert(0);
		// TODO: LOG ERR
		return ret;
	}

	// del from wait2start list after success.
	this->delFrom2Start();

	return 0;
}

int CStaticPrcMngInfo::subStop()
{
	assert(getOppositeEnd());
	// release
	int ret = CRtPrcInfo::sRelease(*getOppositeEnd());
	if (ret < 0)
	{
		assert (0);
		// TODO: LOG
		return ret;
	}

	// add in list to start
	this->addIn2Start();

	return 0;
}

void CStaticPrcMngInfo::sStart(CProcessBase & extPoll)
{
	for (int i = 0; i < (int)COUNTOF(s_staticPrcList); ++i)
	{
		s_staticPrcList[i].addIn2Start();
	}

	return;
}

void CStaticPrcMngInfo::sTerm4Stop()
{
	for (int i = (int)COUNTOF(s_staticPrcList) - 1; i >= 0; --i)
	{
		CStaticPrcMngInfo & tmp = s_staticPrcList[i];
		tmp.Term4Stop();
	}
}

void CStaticPrcMngInfo::sKill4Stop()
{
	for (int i = (int)COUNTOF(s_staticPrcList) - 1; i >= 0; --i)
	{
		CStaticPrcMngInfo & tmp = s_staticPrcList[i];
		tmp.Kill4Stop();
	}
}

void CStaticPrcMngInfo::sStop(CProcessBase & extPoll)
{
	for (int i = (int)COUNTOF(s_staticPrcList) - 1; i >= 0; --i)
	{
		CStaticPrcMngInfo & tmp = s_staticPrcList[i];
		tmp.stop();
	}

	return;
}
