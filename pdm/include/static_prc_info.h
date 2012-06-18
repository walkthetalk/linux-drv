#pragma once

#include "prc_mng_info.h"

#include "cmm/process/process_base.h"

/*
 * the static process's manage infomation.
 */
class CStaticPrcMngInfo : public CPrcMngInfo
{
public:
	/*
	 * constructor
	 * @pBinName:	the binary's name
	 * @mngID:		for future usage
	 */
	explicit CStaticPrcMngInfo(const char * pBinName, const int mngID) :
		CPrcMngInfo(),
		m_pBinName(pBinName),
		m_mngID(mngID)
	{
		// TODO: Add content
	}

	virtual ~CStaticPrcMngInfo()
	{
		// TODO: Add content
	}
public:
	/*
	 * for start/stop this module
	 */
	static void sStart(CProcessBase & extPoll);
	static void sTerm4Stop(void);
	static void sKill4Stop(void);
	static void sStop(CProcessBase & extPoll);

protected:
	/*
	 * provided for base class
	 */
	virtual int subStart(void);
	virtual int subStop(void);

private:
	//FORBID_COPY_CTOR(CStaticPrcMngInfo);
	FORBID_OP_EQUAL(CStaticPrcMngInfo);
public:
	static const size_t s_nMaxStaticPrc;
private:
	const char * m_pBinName;	// binary name
	const int m_mngID;		// manage ID
};
