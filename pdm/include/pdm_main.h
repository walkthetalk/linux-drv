#pragma once

#include "cmm/process/process_base.h"


class CProcessPdm : public CProcessBase
{
public:
	CProcessPdm(int argc, char * argv[]);

	virtual ~CProcessPdm();
public:
	virtual int Start();
	virtual int Stop();

	const char * getExeName() const
	{
		return m_pName;
	}
private:
	const char * m_pName;
private:
	FORBID_OP_EQUAL(CProcessPdm);
	FORBID_COPY_CTOR(CProcessPdm);
};




