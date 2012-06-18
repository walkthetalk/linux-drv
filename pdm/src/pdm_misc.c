#include <stdlib.h>	// system call
#include <stdio.h>
#include <errno.h>

#include "pdm_misc.h"
#include "slot_converter.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

int pdm_misc_set_self_ip()
{
#ifdef CHIP_SIMULATION
	return 0;
#else
	int ret = 0;
//	int phySlot = get_selfSlotID();
//	if (phySlot != 1)
//	{
//		return -EINVAL;
//	}

	const char * const cmd_setip[] =
	{
		"ifconfig eth1 down;ifconfig eth1 hw ether 0A:0B:0C:0D:0E:06;ifconfig eth1 up;ifconfig eth1 200.100.0.6 netmask 255.255.255.0",
	//	"ifconfig eth1 down;ifconfig eth1 hw ether 0A:0B:0C:0D:0E:07;ifconfig eth1 up;ifconfig eth1 200.100.0.7 netmask 255.255.255.0",
	};

//	int idx = phySlot - 1;
//	if (idx < 0 || ARRAY_SIZE(cmd_setip) <= idx)
//	{
//		return -EINVAL;
//	}

	ret = system(cmd_setip[0]);
	if (ret < 0)
	{
		perror("set ip");
	}

	return ret;
#endif
}
