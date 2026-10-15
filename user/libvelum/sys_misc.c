#include "sys_int.h"
#include "string.h"
#include "velum/vmisc.h"
#include "velum/vobj.h"
#include "velum/vtime.h"

int	v_log(uint32_t level, const char *msg)
{
	size_t	len;

	if (!msg)
		return (E_FAULT);
	len = strnlen(msg, V_LOG_MAX);
	return ((int)sys3(SYS_LOG, level, sys_ptr(msg), len));
}

int64_t	v_getrandom(void *buf, size_t len, uint32_t flags)
{
	return (sys3(SYS_GETRANDOM, sys_ptr(buf), len, flags));
}

int	v_sysinfo(t_sysinfo *out)
{
	return ((int)sys1(SYS_SYSINFO, sys_ptr(out)));
}

int	v_timer_set(t_handle timer, uint64_t deadline_ns, uint64_t period_ns)
{
	return ((int)sys3(SYS_TIMER_SET, timer, deadline_ns, period_ns));
}

int	v_power(uint32_t op)
{
	return ((int)sys1(SYS_POWER, op));
}
