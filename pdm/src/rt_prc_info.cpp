
#include "rt_prc_info.h"
#include "prc_mng_info.h"

#include <sys/types.h>
#include <sys/wait.h>
#include   <errno.h>

#include "pdm_global.h"
#include "utility/omtlog.h"

/*
 * data section
 */
#define SIGHELLOACK SIGUSR1	// hello ack signal number
#define INTERVAL_HELLO 10000	// interval of hello
#define INTERVAL_SPV_HELLO_FACTOR 6	// the factor of checkHello and hello
// the max times of difference between hello and its' ack, I can tolerated.
#define DIFF_SPV_HELLO 5
// interval of checkHello
#define INTERVAL_SPV ((INTERVAL_HELLO)*(INTERVAL_SPV_HELLO_FACTOR))
//interval of kill timeout
#define INTERVAL_KILL INTERVAL_HELLO
#define INTERVAL_TERM INTERVAL_HELLO

// supervision timer for check hello's ack
FD_timer CRtPrcInfo::s_fdSpvTimer;
// hello timer for send hello
FD_timer CRtPrcInfo::s_fdHelloTimer;
// term timer for detect if received SIGCHLD after term
FD_timer CRtPrcInfo::s_fdTermTimer;
// kill timer for detect if received SIGCHLD after kill
FD_timer CRtPrcInfo::s_fdKillTimer;
// signal detector for SIGCHLD
FD_signal CRtPrcInfo::s_fdSigChld;
// signal detector for SIGHELLOACK
FD_signal CRtPrcInfo::s_fdSigHelloAck;

// pre-allocated *CRtPrcInfo* pool
CRtPrcInfo CRtPrcInfo::s_preAllPool[ETIN_SLOT_MAX + MAX_STATIC_PRC_NUM];
// the list contain all pre-allocated *CRtPrcInfo*
CListHead<CRtPrcInfo> CRtPrcInfo::s_listPool;
// the list contain all normal state *CRtPrcInfo*
CListHead<CRtPrcInfo> CRtPrcInfo::s_listNormal;
// the list contain all exiting state *CRtPrcInfo*
CListHead<CRtPrcInfo> CRtPrcInfo::s_listExiting;
// the list contain all killing state *CRtPrcInfo*
CListHead<CRtPrcInfo> CRtPrcInfo::s_listKilling;


/*
 * code section
 */

/*
 * get current state
 */
EPrcRtState_t CRtPrcInfo::getState() const
{
	return m_state;
}

/*
 * start a new process
 * @pExeName:	the binary name.
 * @argv:		the arguments of the process
 */
int CRtPrcInfo::start(const char * pExeName,
		const char * const argv[])
{
	assert(m_state == EPRC_RT_NULL);
	if (pExeName == NULL)
		//|| m_state != EPRC_RT_NULL)
	{
		assert(0);
		return -1;
	}

	assert(m_pid == -1);
	assert(m_nExpTimes == 0);
	assert(m_nRealTimes == 0);

	int newPid = fork();
	if (newPid == 0)
	{
		std::cout << "start exeName: ";
		int i = 0;
		while (argv[i] != NULL)
		{
			std::cout << " " << argv[i];
			++i;
		}
		std::cout << std::endl;
		int exeRet = execvp(pExeName, (char * *)argv);	// TODO: this conversion is not right solution
		// if execvp returned, there is an error occurred.
		// TODO: resume CHECK_ERR(exeRet, pExeName);
		exit(exeRet);
	}
	else if (newPid < 0)
	{
		// fail to fork child process
		// TODO: LOG errno
		return newPid;
	}

	// success
	m_state = EPRC_RT_NORMAL;
	m_pid = newPid;

	m_argv_str = "";
	int i = 0;
	while (argv[i] != NULL)
	{
		m_argv_str += argv[i];
		m_argv_str += " ";
		++i;
	}

	OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,"process %s start, pid : %d \n",m_argv_str.c_str(), m_pid);

	return 0;
}

/*
 * stop the process
 * NOTE: only wait process's exit and reclaim it.
 */
int CRtPrcInfo::stop()
{
	assert(m_state == EPRC_RT_NORMAL		// initiative exit
		|| m_state == EPRC_RT_EXITING		// passive exit
		|| m_state == EPRC_RT_KILLING);	// passive exit

	int status = 0;
	pid_t retPid = waitpid(m_pid, &status, WNOHANG);
	//if waitpid fail fobid modifying data
	if ((retPid != m_pid)&&!((retPid==-1)&&(ECHILD == errno)))
	{
		assert(0);
		// TODO: LOG
		return -1;
	}


	OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,"process %s stop, pid : %d\n",m_argv_str.c_str(),m_pid);
	m_argv_str = "";
	m_state = EPRC_RT_NULL;
	m_pid = -1;

	return 0;
}

/*
 * send hello to the process
 * NOTE: currently not send any info, but only update the expected
 * count of helloAck.
 */
inline void CRtPrcInfo::sendHello()
{
	assert(m_state == EPRC_RT_NORMAL);
	updateExpTimes();

	return;
}

/*
 * update expected count. (timer use it)
 */
inline void CRtPrcInfo::updateExpTimes()
{
	++m_nExpTimes;

	return;
}

/*
 * update real count. (timer use it)
 */
inline void CRtPrcInfo::updateRealTimes()
{
	++m_nRealTimes;

	// TODO: del
	//std::cout << m_nRealTimes << std::endl;

	return;
}

/*
 * clear all count. (timer use it)
 */
inline void CRtPrcInfo::clrTimes()
{
	m_nExpTimes = 0;
	m_nRealTimes = 0;

	return;
}

/*
 * check if the process is live.
 * @diff:	the max number of difference between expected and real count I can tolerate.
 */
inline bool CRtPrcInfo::isLive(int diff) const
{
	return (((m_nExpTimes - m_nRealTimes) < diff)
		? true : false);
}

/*
 * send SIGKILL to the process to kill it.
 */
void CRtPrcInfo::signalKill()
{
	assert(m_state == EPRC_RT_NORMAL
		|| m_state == EPRC_RT_EXITING);

	OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,"I want to kill %d\n", m_pid);
	int ret = kill(m_pid, SIGKILL);
	CHECK_ERR(ret, "kill");

	m_state = EPRC_RT_KILLING;

	return;
}

/*
 * send SIGTERM to the process to kill it.
 */
void CRtPrcInfo::signalTerm()
{
	assert(m_state == EPRC_RT_NORMAL
		|| m_state == EPRC_RT_EXITING);

	OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,"I want to exit %d\n", m_pid);
	int ret = kill(m_pid, SIGTERM);
	CHECK_ERR(ret, "term");

	m_state = EPRC_RT_EXITING;

	return;
}

/*
 * start the module
 */
void CRtPrcInfo::sStart(CProcessBase & extPoll)
{
	sInitPool();

	s_fdSigHelloAck = extPoll.RegisterSignal(SIGHELLOACK, CRtPrcInfo::sUpdateLiveState, NULL);

	s_fdHelloTimer = extPoll.NewTimer("sHello4Live",CRtPrcInfo::sHello4Live, INTERVAL_HELLO, NULL);

	// if debug version, don't check live state
	s_fdSpvTimer = extPoll.NewTimer("sCheckLiveState",CRtPrcInfo::sCheckLiveState, INTERVAL_SPV, NULL);

	s_fdTermTimer = extPoll.NewTimer("sCheckTermTimeout",CRtPrcInfo::sCheckTermTimeout, INTERVAL_TERM, NULL);

	s_fdKillTimer = extPoll.NewTimer("sCheckKillTimeout",CRtPrcInfo::sCheckKillTimeout, INTERVAL_KILL, NULL);

	s_fdSigChld = extPoll.RegisterSignal(SIGCHLD, CRtPrcInfo::sBuryChld, NULL);

	//

	return;
}

/*
 * stop the module
 */
void CRtPrcInfo::sStop(CProcessBase & extPoll)
{
	//
	extPoll.UnRegisterSignal(s_fdSigChld);

	extPoll.StopTimer(s_fdKillTimer);
	extPoll.DeleteTimer(s_fdKillTimer);

	extPoll.StopTimer(s_fdTermTimer);
	extPoll.DeleteTimer(s_fdTermTimer);

	extPoll.StopTimer(s_fdSpvTimer);
	extPoll.DeleteTimer(s_fdSpvTimer);

	extPoll.StopTimer(s_fdHelloTimer);
	extPoll.DeleteTimer(s_fdHelloTimer);

	extPoll.UnRegisterSignal(s_fdSigHelloAck);
}

/*
 * timer's callback, to check all normal process's live state.
 * if one is not live, then kill it.
 */
int CRtPrcInfo::sCheckLiveState(void * pArg)
{
//	std::cout << "check live state" << std::endl;
	listForEachSafe(pObj, s_listNormal)
	{
		CRtPrcInfo * pInst = (CRtPrcInfo *)pObj;

		// check live state
		bool bLive = pInst->isLive(DIFF_SPV_HELLO);
		if (bLive)
		{
			pInst->clrTimes();
			continue;
		}

		// kill the process
		sKill(*pInst);
	}

	return 0;
}

/*
 * timer's callback to send hello to all normal process.
 */
int CRtPrcInfo::sHello4Live(void * pArg)
{
	listForEachSafe(pObj, s_listNormal)
	{
		CRtPrcInfo * pInst = (CRtPrcInfo *)pObj;

		// update expHb
		pInst->sendHello();
	}

	return 0;
}

/*
 * timer's call-back to check term's timeout.
 */
int CRtPrcInfo::sCheckTermTimeout(void * pArg)
{
	CRtPrcInfo * pInst = s_listExiting.getFirstEntry();
	if (pInst)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),	\
				OMT_LOG_INFO,"warning: Term timeout for process %d\n", pInst->getPid());
		// TODO: LOG WARNING
		CRtPrcInfo::sKill(*pInst);
	}

	if (s_listExiting.isEmpty())
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,"stop term timer\n");
		CPdmGlobal::s_pPdmPrcBase->StopTimer(s_fdTermTimer);
	}

	return 0;
}

/*
 * timer's call-back to check kill's timeout.
 */
int CRtPrcInfo::sCheckKillTimeout(void * pArg)
{
	CRtPrcInfo * pInst = s_listKilling.getFirstEntry();
	if (pInst)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"warning: kill timeout for process %d\n",pInst->getPid());
		// TODO: LOG WARNING
		CPrcMngInfo * pMngInfo = pInst->getOppositeEnd();
		if (pMngInfo)
		{
			//无论stop操作失败或者成功，此函数内不可有数据操作
			pMngInfo->stop();
		}
		else
		{
			assert(0);
			// TODO: LOG
		}
	}

	if (s_listKilling.isEmpty())
	{
		std::cout << "stop kill timer" << std::endl;
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,"stop kill timer\n");
		CPdmGlobal::s_pPdmPrcBase->StopTimer(s_fdKillTimer);
	}

	return 0;
}

/*
 * tool function, to find the expected process from one list.
 * @head:	the list may contain expected process
 * @pid:	the expected process's pid.
 * if can't find it, then return NULL.
 */
CRtPrcInfo * CRtPrcInfo::sFindObj(CListHead<CRtPrcInfo> & head, pid_t pid)
{
	listForEach(pObj, head)
	{
		CRtPrcInfo * pInst = (CRtPrcInfo *)pObj;
		if (pInst->m_pid == pid)
		{
			return pInst;
		}
	}
	return NULL;
}

/*
 * timer's callback, bury the child process.
 * find the process and restart it.
 */
int CRtPrcInfo::sBuryChld(const struct signalfd_siginfo & siginfo, void * pArg)
{
	//std::cout << "code is " << siginfo.ssi_signo << std::endl;
	assert(siginfo.ssi_signo == SIGCHLD);

	pid_t w_pid = -1;
	int status = 0;
	while((w_pid = waitpid(-1, &status, WNOHANG))>0)
	{
		// 1. find it
		CRtPrcInfo * pObj = NULL;

		pObj = sFindObj(s_listKilling, w_pid);
		if (pObj == NULL)
		{
			pObj = sFindObj(s_listExiting, w_pid);
		}

		if (pObj == NULL)
		{
			pObj = sFindObj(s_listNormal, w_pid);
		}

		if (pObj == NULL)
		{
			OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
					"I can't recognize the pid %d \n",w_pid);
			continue;
		}

		// 2. stop it.
		CPrcMngInfo * pMngInfo = pObj->getOppositeEnd();
		if (pMngInfo)
		{
			pMngInfo->stop();
		}
		else
		{
			assert(0);
			// TODO: LOG
		}
	}
	return 0;
}

/*
 * timer's call-back, record helloAck
 */
int CRtPrcInfo::sUpdateLiveState(const struct signalfd_siginfo & siginfo, void * pArg)
{
	assert(siginfo.ssi_signo == SIGHELLOACK);

	//std::cout << "rcved sigHelloack" << std::endl;

//#define SELF_TEST
#ifdef SELF_TEST
	listForEach(pObj, s_listNormal)
	{
		CRtPrcInfo * pInst = (CRtPrcInfo *)pObj;
		pInst->updateRealTimes();
	}
#else
	CRtPrcInfo * pObj = NULL;
	pObj = sFindObj(s_listNormal, (pid_t)siginfo.ssi_pid);
	if (pObj == NULL)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"warning: rcved a helloack from the process %d , but it is not in normal list. \n",(pid_t)siginfo.ssi_pid);
		// TODO: LOG
		return -1;
	}

	pObj->updateRealTimes();
#endif

	return 0;
}

/*
 * put all pre-allocated resources into the list *s_listPool* for future use.
 */
void CRtPrcInfo::sInitPool()
{
	for (size_t i = 0; i < COUNTOF(s_preAllPool); ++i)
	{
		s_listPool.addTail(&s_preAllPool[i]);
	}
}

/*
 * capture one resource and start one process, return it.
 * state change: NULL->NORMAL
 */
int CRtPrcInfo::sCapture(CPrcMngInfo & parent,
	const char * pExeName,
	const char * const argv[])
{
	assert(!s_listPool.isEmpty());
	CRtPrcInfo * pInst = s_listPool.getFirstEntry();
	if (pInst == NULL)
	{
		assert(0);
		return -1;
	}

	int exeRet = pInst->start(pExeName, argv);
	if (exeRet < 0)
	{
		assert(0);
		return -1;
	}

	buildBdAss(parent, *pInst);

	// move
	s_listNormal.accept(pInst);

	return 0;
}

/*
 * release one resource and reclaim it.
 * state change: *->NULL
 */
int CRtPrcInfo::sRelease(CRtPrcInfo & rhs)
{
	int ret = rhs.stop();
	//if stop fail fobid modifying data
	if (ret < 0)
	{
		assert(0);
		return ret;
	}

	destroyBdAss(*rhs.getOppositeEnd(), rhs);
	rhs.clrTimes();

	// move
	s_listPool.accept(&rhs);

	return 0;
}

/*
 * term one process
 * state change: *->Exiting
 */
void CRtPrcInfo::sTerm(CRtPrcInfo & rhs)
{
	rhs.signalTerm();
	rhs.clrTimes();

	if (s_listExiting.isEmpty())
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"warning: start term timer for %d \n",rhs.getPid());

		CPdmGlobal::s_pPdmPrcBase->StartTimer(s_fdTermTimer);
	}
	else
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
				"warning: term multi process, you may lost SIGCHLD of %d \n",rhs.getPid());

	}

	s_listExiting.accept(&rhs);

	return;
}

/*
 * kill one process
 * state change: *->Killing
 */
void CRtPrcInfo::sKill(CRtPrcInfo & rhs)
{

	if (!CPdmGlobal::s_needKillPrc)
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
						"error: want to kill pid %d , but skipped that\n",rhs.getPid());
		return;
	}

	rhs.signalKill();
	rhs.clrTimes();

	if (s_listKilling.isEmpty())
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
						"warning: start kill timer for %d\n",rhs.getPid());

		CPdmGlobal::s_pPdmPrcBase->StartTimer(s_fdKillTimer);
	}
	else
	{
		OMT_SYSLOG(CPdmGlobal::s_pPdmPrcBase->GetModuleID(),OMT_LOG_INFO,	\
						"warning: kill multi process, you may lost SIGCHLD of %d\n",rhs.getPid());
	}

	s_listKilling.accept(&rhs);

	return;
}




