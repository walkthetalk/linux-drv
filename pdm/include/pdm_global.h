#pragma once

#include "pdm_main.h"

class CPdmGlobal
{
public:
	static bool s_needKillPrc;
	static CProcessPdm * s_pPdmPrcBase;
public:
	static const char * getExeName()
	{
		return s_pPdmPrcBase ? s_pPdmPrcBase->getExeName() : NULL;
	}
};


