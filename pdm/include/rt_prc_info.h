#pragma once

#include <unistd.h>

#include "bd_ass.h"
#include "cplist.h"

#include "cmm/process/process_base.h"

typedef int mngID_t;
typedef enum
{
	EPRC_RT_NULL = 0x0,
	EPRC_RT_NORMAL,
	EPRC_RT_EXITING,
	EPRC_RT_KILLING,
} EPrcRtState_t;

class CPrcMngInfo;
/*
 * the realtime infomation of one process. it will always connect with
 * one *CPrcMngInfo*.
 */
class CRtPrcInfo : public CListHeader, public CBdAssChild<CPrcMngInfo>
{
public:
	explicit CRtPrcInfo() :
		CListHeader(),
		CBdAssChild<CPrcMngInfo>(),
		m_state(EPRC_RT_NULL),
		m_pid(-1),
		m_nExpTimes(0),
		m_nRealTimes(0)
	{
		// TODO:
	}

	virtual ~CRtPrcInfo()
	{
		// TODO:
	}
public:
	pid_t getPid(void)
	{
		return m_pid;
	}
public:
	/*
	 * interfaces for start/stop this module
	 * @extPoll:	all pollObj should be added in it.
	 */
	static void sStart(CProcessBase & extPoll);
	static void sStop(CProcessBase & extPoll);

public:
	/*
	 *  use these ifs to control the prc's state transfer
	 */
	/*
	 * capture one object, use it represent one process.
	 * @parent:		manage infomation
	 * @pExeName:	the binary program's name
	 * @argv:		the arguments passed to the binary
	 */
	static int sCapture(CPrcMngInfo & parent,
		const char * pExeName,
		const char * const argv[]);
	/*
	 * term one process
	 * @rhs:	the process will be terminated
	 */
	static void sTerm(CRtPrcInfo & rhs);
	/*
	 * kill one process
	 * @rhs:	the process will be killed
	 */
	static void sKill(CRtPrcInfo & rhs);
	/*
	 * release one process related things.
	 * @rhs:	the process will be reclaimed
	 */
	static int sRelease(CRtPrcInfo & rhs);

public:
	EPrcRtState_t getState() const;
	void signalKill();
	void signalTerm();
private:
	int start(const char * pExeName,
		const char * const argv[]);
	void sendHello(void);
	int stop();
private:
	void updateExpTimes(void);
	void updateRealTimes(void);
	bool isLive(int diff) const;
	void clrTimes(void);

private:
	static int sCheckLiveState(void * pArg);
	static int sHello4Live(void * pArg);
	static int sCheckTermTimeout(void * pArg);
	static int sCheckKillTimeout(void * pArg);
	static int sBuryChld(const struct signalfd_siginfo & siginfo, void * pArg);
	static int sUpdateLiveState(const struct signalfd_siginfo & siginfo, void * pArg);

private:
	static void sInitPool(void);
	static CRtPrcInfo * sFindObj(CListHead<CRtPrcInfo> & head, pid_t pid);
private:
	FORBID_COPY_CTOR(CRtPrcInfo);
	FORBID_OP_EQUAL(CRtPrcInfo);

private:
	static FD_timer s_fdSpvTimer;
	static FD_timer s_fdHelloTimer;
	static FD_timer s_fdTermTimer;
	static FD_timer s_fdKillTimer;
	static FD_signal s_fdSigChld;
	static FD_signal s_fdSigHelloAck;
private:
	static CRtPrcInfo s_preAllPool[];
	static CListHead<CRtPrcInfo> s_listPool;

	static CListHead<CRtPrcInfo> s_listNormal;
	static CListHead<CRtPrcInfo> s_listExiting;
	static CListHead<CRtPrcInfo> s_listKilling;
private:
	//struct list_head	head;
	//const mngID_t	m_mngID;
	EPrcRtState_t	m_state;
	pid_t 		m_pid;		// pid of the process
	string		m_argv_str;
private:	// used by timer
	int		m_nExpTimes;	// expect number
	int		m_nRealTimes;	// real number
	//...
};

