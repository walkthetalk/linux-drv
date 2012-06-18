#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "rt_prc_info.h"
#include "static_prc_info.h"
#include "dynamic_prc_info.h"
#include "pdm_misc.h"

#include "pdm_main.h"

#include "pdm_global.h"
#include "utility/omtlog.h"
#include "cmm/moduledefine.h"

#include "modules/create_prd_process.h"

CProcessBase *CreateProcessPdm(int argc, char * argv[])
{
	return new CProcessPdm(argc, argv);
}


#ifdef NDEBUG
bool CPdmGlobal::s_needKillPrc = true;
#else
bool CPdmGlobal::s_needKillPrc = false;
#endif

CProcessPdm * CPdmGlobal::s_pPdmPrcBase = NULL;



CProcessPdm::CProcessPdm(int argc, char * argv[])
	: CProcessBase(argc, argv)
	, m_pName(argv[0])
{

	int opt = 0;
	while ((opt = getopt(argc, argv, "k::")) != -1)
	{
		switch (opt)
		{
		case 'k':
			if (optarg == NULL)
			{
				CPdmGlobal::s_needKillPrc = true;
			}
			else if (strcmp(optarg, "true") == 0)
			{
				CPdmGlobal::s_needKillPrc = true;
			}
			else if (strcmp(optarg, "false") == 0)
			{
				CPdmGlobal::s_needKillPrc = false;
			}
			else
			{
				OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
								"the arg of -k option must be \"ture\" or \"false\\n");
				exit(-EINVAL);
			}
			break;
		}
	}

	m_moduleid = SetModuleID(TIN_INTF_MESSAGE::EModuleType_DEVMNG,SetModuleInstance(m_slotNum,0));

	return;
}

CProcessPdm::~CProcessPdm()
{
	return;
}

int CProcessPdm::Start()
{
	int ret = ERet_cmm_err;
	ret = CProcessBase::Start();
	if (ret)
	{
		return ret;
	}

	CPdmGlobal::s_pPdmPrcBase = this;

	CRtPrcInfo::sStart(*this);
	CPrcMngInfo::sStart(*this);
	CStaticPrcMngInfo::sStart(*this);
	CDynamicPrcMngInfo::sStart(*this);

/*
	int setipret = pdm_misc_set_self_ip();
	if (setipret < 0)
	{
		ret = ERet_cmm_err;
	}
*/
	return ret;
}

int CProcessPdm::Stop()
{
	INT ret = ERet_cmm_err;

	CDynamicPrcMngInfo::sTerm4Stop();
	CStaticPrcMngInfo::sTerm4Stop();
	sleep(2);
	CDynamicPrcMngInfo::sKill4Stop();
	CStaticPrcMngInfo::sKill4Stop();
	sleep(1);

	CDynamicPrcMngInfo::sStop(*this);
	CStaticPrcMngInfo::sStop(*this);
	CPrcMngInfo::sStop(*this);
	CRtPrcInfo::sStop(*this);

	ret = CProcessBase::Stop();

	return ret;
}


