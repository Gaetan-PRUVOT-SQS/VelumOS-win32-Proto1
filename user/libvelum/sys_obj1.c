#include "sys_int.h"
#include "velum/vobj.h"

int	v_close(t_handle h)
{
	return ((int)sys1(SYS_CLOSE, h));
}

int64_t	v_dup(t_handle h, uint32_t rights)
{
	return (sys2(SYS_DUP, h, rights));
}

int	v_wait(t_handle h, uint64_t timeout_ns)
{
	return ((int)sys2(SYS_WAIT, h, timeout_ns));
}

int	v_wait_many(const t_handle *handles, uint32_t n, uint32_t flags,
		uint64_t timeout_ns)
{
	uint64_t	a[V_SYS_ARGS];

	a[0] = sys_ptr(handles);
	a[1] = n;
	a[2] = flags;
	a[3] = timeout_ns;
	a[4] = 0;
	a[5] = 0;
	return ((int)v_syscall6(SYS_WAIT_MANY, a));
}

int64_t	v_event_create(int manual, int initial)
{
	return (sys2(SYS_EVENT_CREATE, manual != 0, initial != 0));
}
