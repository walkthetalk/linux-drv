
//#include <unistd.h>
#include <time.h>

#include "ukc_types.h"
#include "ukc_delay.h"

#define NSEC_PER_SEC 1000000000

static inline void set_normalized_timespec(struct timespec *ts, time_t sec, int64_t nsec)
{
	while (nsec >= NSEC_PER_SEC) {
		/*
		 * The following asm() prevents the compiler from
		 * optimising this loop into a modulo operation. See
		 * also __iter_div_u64_rem() in include/linux/time.h
		 */
		asm("" : "+rm"(nsec));
		nsec -= NSEC_PER_SEC;
		++sec;
	}
	while (nsec < 0) {
		asm("" : "+rm"(nsec));
		nsec += NSEC_PER_SEC;
		--sec;
	}
	ts->tv_sec = sec;
	ts->tv_nsec = nsec;
}

static inline struct timespec timespec_add_safe(const struct timespec lhs,
				  const struct timespec rhs)
{
	struct timespec res;

	set_normalized_timespec(&res, lhs.tv_sec + rhs.tv_sec,
				lhs.tv_nsec + rhs.tv_nsec);
#if 0	// I don't think our device will running 68 year continuously.
	if (res.tv_sec < lhs.tv_sec || res.tv_sec < rhs.tv_sec)
		res.tv_sec = TIME_T_MAX;
#endif

	return res;
}

static inline void ukc_sleep_internal(unsigned int seconds, unsigned int nanoseconds)
{
	struct timespec ts;
	struct timespec ts_cur;
	struct timespec ts_inc = { .tv_sec = seconds, .tv_nsec = nanoseconds, };
	int ret = 0;

	ret = clock_gettime(CLOCK_MONOTONIC, &ts_cur);
	if (ret < 0)
	{
		// TODO:
		return;
	}

	ts = timespec_add_safe(ts_cur, ts_inc);

	do
	{
		ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
	} while (ret < 0);

	return;
}

void ukc_ssleep(unsigned int seconds)
{
#if 0
	unsigned int ret = seconds;
	while (ret > 0)
	{
		ret = sleep(seconds);
	}
#else
	ukc_sleep_internal(seconds, 0);
#endif

	return;
}

void ukc_mssleep(unsigned int milliseconds)
{
	ukc_sleep_internal(milliseconds/1000, milliseconds%1000 * 1000000);

	return;
}

void ukc_ussleep(unsigned int microseconds)
{
	ukc_sleep_internal(microseconds/1000000, microseconds%1000000 * 1000);

	return;
}

void ukc_nssleep(unsigned int nanoseconds)
{
	ukc_sleep_internal(nanoseconds/1000000000, nanoseconds%1000000000);

	return;
}


