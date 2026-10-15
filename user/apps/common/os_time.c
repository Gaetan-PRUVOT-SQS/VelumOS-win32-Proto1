#include "velum/velum.h"
#include "platform.h"

uint64_t	os_mono_ns(void)
{
	int64_t	t;

	t = v_time_mono();
	if (t < 0)
		return (0);
	return ((uint64_t)t);
}

uint64_t	os_wall_ns(void)
{
	int64_t	t;

	t = v_time_wall();
	if (t < 0)
		return (0);
	return ((uint64_t)t);
}

int	os_sleep_ns(uint64_t ns)
{
	return (v_sleep(ns));
}
