#pragma once

#include <unistd.h>	//close

#include <stdio.h>	//perror
#include <stdlib.h>	//exit
#include <assert.h>

#include <iostream>

#include "driver/drv_cmm_hdr.h"

#include "pdm_global.h"

#ifdef __PDM_SEP_POLL__
/*
 * get the member's count of an array
 * @x:		the name of the array
 */
#define COUNTOF(x) (sizeof(x) / sizeof(x[0]))

/*
 * check some result if error occurred.
 * @ret:	the result be checked
 * @pInfo:	the information with the *ret*
 * if *ret* less than *0*, it will print out the error infomation, and then exit.
 */
inline void check_err(int ret, const char * pInfo)
{
	if (ret < 0)
	{
		//assert(0);
		perror(pInfo);
		exit(-1);
	}
}

#define CHECK_ERR(ret, pInfo) check_err((ret), (pInfo))

/*
 * you can use these macros to declare some member function of one class.
 * NOTE: you should declare it as *private*.
 */
/*
 * override *operator =*
 */
#define FORBID_OP_EQUAL(x) x & operator = (const x & rhs)
/*
 * copy constructor. (the parameter is *const*)
 */
#define FORBID_COPY_CTOR(x) x(const x & rhs)

#else

#include "epoll/poll_tools.h"

#endif

/*
 * max device number. (used by device management)
 */
//#define MAX_DEV_NUM ETIN_SLOT_MAX
/*
 * max slot number. (maybe some slot is not under my control, so...)
 */
//#define PDM_MAX_SLOT_NUM ETIN_SLOT_MAX
/*
 * max static process number (started with system).
 */
#define MAX_STATIC_PRC_NUM 5

/*
 * the program's name
 */
#define TIN_EXE_NAME (CPdmGlobal::getExeName())
/*
 * the device file of cpld
 */
#define CPLD_DEV_FILE_PATH "/dev/cpld0"

