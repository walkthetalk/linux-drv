/********************************************************************
	created:	2010/09/17
	filename: 	process_hrm.h
	file path:	include
	file base:	${ROOT}/pdm
	file ext:	h
	author:		Ni Qingliang

	purpose:	pdm module header file
*********************************************************************/
#pragma once

#include "cmm/module_base/module_base.h"


/*
 * self-use header
 */
#include "rt_prc_info.h"
#include "static_prc_info.h"
#include "dynamic_prc_info.h"

class CProcessPdm:public CModuleBase
{
public:
	CProcessPdm(int argc, char ** argv)
		: CModuleBase(argc, argv)
	{
	}
	virtual ~CProcessPdm()
	{
	}
	virtual int Start()
	{
		CRtPrcInfo::sStart(m_extPoll);
		CPrcMngInfo::sStart(m_extPoll);
		CStaticPrcMngInfo::sStart(m_extPoll);
		CDynamicPrcMngInfo::sStart(m_extPoll);


		int ret = ERet_cmm_success;
		if(TIN_INTF_MESSAGE::EHrmStatus_slave != m_hrmstatus)
		{
			m_pReConnectTimerSchedule = newTimerSchedule(1,Reconnect,1000,true,this);
		}
		//对端的DE不一定能练上，需要启动定时器来做这个事情
		if(TIN_INTF_MESSAGE::EHrmStatus_master != m_hrmstatus)
		{
			m_pReConnectOppDeTimerSchedule = newTimerSchedule(1,ReconnectBack,1000,true,this);
		}

		return ERet_cmm_success;
	};

	virtual int Stop()
	{
		int ret = CModuleBase::Stop();
		CDynamicPrcMngInfo::sStop(m_extPoll);
		CStaticPrcMngInfo::sStop(m_extPoll);
		CPrcMngInfo::sStop(m_extPoll);
		CRtPrcInfo::sStop(m_extPoll);
		return ret;
	};

	virtual void ProcessEntry()
	{
		m_extPoll.runOnce();
		CModuleBase::runOnce();
	}
private:
	CExtPoll m_extPoll;
};



