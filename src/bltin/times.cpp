/*
 * Copyright (c) 1999 Herbert Xu <herbert@gondor.apana.org.au>
 * This file contains code for the times builtin.
 */

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif

#ifdef USE_GLIBC_STDIO
#include <stdio.h>
#else
#include "bltin.h"
#endif
#include "../system.h"

#ifdef _MSC_VER
static unsigned long long ticks2minutes(unsigned long long x)
{
	// x is in units of 100ns.  There are 10^7 ticks per second.
	return (x / 10000000) / 60;
}

static double ticks2seconds_remainder(unsigned long long x)
{
	return (static_cast<double>(x) / 10000000.0 - ticks2minutes(x) * 60.0);
}

static unsigned long long unpack_filetime(FILETIME ft)
{
	unsigned long long x;
	x = static_cast<unsigned long long>(ft.dwHighDateTime) << 32;
	x += static_cast<unsigned long long>(ft.dwLowDateTime);
	return x;
}

int timescmd(int argc, char **argv) {

	FILETIME creation_time, exit_time, kernel_time, user_time;
	if ((GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time) == 0)) {
		// Failed to get process times
		printf("%dm%fs %dm%fs\n%dm%fs %dm%fs\n", 0, 0.0, 0, 0.0, 0, 0.0, 0, 0.0);
	}
	else {
		unsigned long long ll_utime = unpack_filetime(user_time);
		unsigned long long ll_ktime = unpack_filetime(kernel_time);
		unsigned long long ll_etime = unpack_filetime(exit_time);

		printf("%llum%fs %llum%fs\n%llum%fs %llum%fs\n",
			ticks2minutes(ll_utime),
			ticks2seconds_remainder(ll_utime),
			ticks2minutes(ll_ktime),
			ticks2seconds_remainder(ll_ktime),
			ticks2minutes(ll_etime),
			ticks2seconds_remainder(ll_etime),
			ticks2minutes(ll_ktime),
			ticks2seconds_remainder(ll_ktime));
	}
	return 0;
}

#else
int timescmd(int argc, char **argv) {
	struct tms buf;
	long int clk_tck = sysconf(_SC_CLK_TCK);

	times(&buf);
	printf("%dm%fs %dm%fs\n%dm%fs %dm%fs\n",
		(int)(buf.tms_utime / clk_tck / 60),
		((double)buf.tms_utime) / clk_tck,
		(int)(buf.tms_stime / clk_tck / 60),
		((double)buf.tms_stime) / clk_tck,
		(int)(buf.tms_cutime / clk_tck / 60),
		((double)buf.tms_cutime) / clk_tck,
		(int)(buf.tms_cstime / clk_tck / 60),
		((double)buf.tms_cstime) / clk_tck);
	return 0;
}
#endif
