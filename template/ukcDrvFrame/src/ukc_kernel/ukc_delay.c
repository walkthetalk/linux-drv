#include <linux/delay.h>

#include "ukc_delay.h"


void ukc_ssleep(unsigned int seconds)
{
	ssleep(seconds);

	return;
}

void ukc_mssleep(unsigned int milliseconds)
{
	msleep(milliseconds);

	return;
}

void ukc_ussleep(unsigned int microseconds)
{
	udelay(microseconds);

	return;
}

void ukc_nssleep(unsigned int nanoseconds)
{
	ndelay(nanoseconds);

	return;
}
