#include "sys_int.h"
#include "velum/vproc.h"

_Noreturn void	v_thread_exit_raw(int code)
{
	sys1(SYS_THREAD_EXIT, (uint64_t)(int64_t)code);
	__builtin_trap();
}

int	v_thread_prio(t_handle thread, uint32_t prio)
{
	return ((int)sys2(SYS_THREAD_PRIO, thread, prio));
}

int64_t	v_gettid(void)
{
	return (sys0(SYS_GETTID));
}

int	v_set_fsbase(uint64_t base)
{
	return ((int)sys1(SYS_SET_FSBASE, base));
}

int	v_proc_list(t_procinfo *out, uint32_t max)
{
	return ((int)sys2(SYS_PROC_LIST, sys_ptr(out), max));
}
