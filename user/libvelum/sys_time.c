#include "sys_int.h"
#include "velum/vtime.h"

int	v_yield(void)
{
	return ((int)sys0(SYS_YIELD));
}

int	v_sleep(uint64_t ns)
{
	return ((int)sys1(SYS_SLEEP, ns));
}

int64_t	v_time_mono(void)
{
	return (sys0(SYS_TIME_MONO));
}

int64_t	v_time_wall(void)
{
	return (sys0(SYS_TIME_WALL));
}

int	v_time_set_wall(uint64_t ns)
{
	return ((int)sys1(SYS_TIME_SET_WALL, ns));
}
