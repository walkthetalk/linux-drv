#pragma once

#include "bd_ass.h"
#include "cplist.h"

#include "cmm/process/process_base.h"

class CRtPrcInfo;
/*
 * process manager infomation.
 * it will connect with one *CRtPrcInfo* when the process is created.
 */
class CPrcMngInfo : public CBdAssParent<CRtPrcInfo>, public CListHeader
{
public:
	explicit CPrcMngInfo() :
		CBdAssParent<CRtPrcInfo>()
	{
		// TODO:
	}

	virtual ~CPrcMngInfo()
	{
		// TODO:
	}
public:
	static void sStart(CProcessBase & extPoll);
	static void sStop(CProcessBase & extPoll);
public:
	void start();
	void Term4Stop(void);
	void Kill4Stop(void);
	void stop();
protected:
	virtual int subStart() = 0;
	virtual int subStop() = 0;
protected:
	void addIn2Start(void)
	{
		if (s_list2Start.isEmpty())
		{
			CPdmGlobal::s_pPdmPrcBase->StartTimer(s_fdTimerStart);
		}
		s_list2Start.addTail(this);
	}
	void delFrom2Start(void)
	{
		this->del();
	}
private:
	static int sCheckNeedStart(void * pArg);
private:
	static FD_timer s_fdTimerStart;
	static CListHead<CPrcMngInfo> s_list2Start;
private:
	//FORBID_COPY_CTOR(CPrcMngInfo);
	FORBID_OP_EQUAL(CPrcMngInfo);
};
