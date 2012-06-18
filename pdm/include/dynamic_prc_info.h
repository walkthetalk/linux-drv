#pragma once

#include "prc_mng_info.h"

#include "cmm/process/process_base.h"

#include "driver/drv_cmm_hdr.h"
#include "driver/pdm_drv_usr.h"

/*
 * the dynamic process's manage information.
 */
class CDynamicPrcMngInfo : public CPrcMngInfo
{
public:
	/*
	 * constructor
	 * @pBinName:	the binary's name
	 * @mngID:	for future usage
	 */
	explicit CDynamicPrcMngInfo(int slotID, const int mngID);

	virtual ~CDynamicPrcMngInfo()
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

	static bool m_sExitFlag;

protected:
	virtual int subStart(void);
	virtual int subStop(void);
private:
	static int sCheckDevChange(void * pArg);
	static int sHandleCardChange(SDevMngDrvData_t * pNewContent, void * pArg);
private:
	void handleNewType(ETinDev_t newCardType);
	int getSlotID(void) const;
	bool haveCardInserted(void) const;
private:
	//FORBID_COPY_CTOR(CDynamicPrcMngInfo);
	FORBID_OP_EQUAL(CDynamicPrcMngInfo);

public:
	static const size_t s_nMaxDev;
private:
	static FD_timer s_fdSpvTimer;
	static FD_file s_fdCPLD;
private:
	const int m_slotID;		// slot id
	ETinDev_t m_cardType;		// card type
	ETinDev_t m_cardTypeNew;	// new card type
	int m_cnt4Filter;		// online filter
	const int m_mngID;		// manage ID

};
