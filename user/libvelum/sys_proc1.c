#include "sys_int.h"
#include "velum/vproc.h"

_Noreturn void	v_exit(int code)
{
	sys1(SYS_EXIT, (uint64_t)(int64_t)code);
	__builtin_trap();
}

int64_t	v_spawn(const char *path, const char *args, size_t args_len,
		uint32_t flags)
{
	uint64_t	a[V_SYS_ARGS];
	int64_t		path_len;

	path_len = sys_cstrlen(path, VFS_PATH_MAX);
	if (path_len < 0)
		return (path_len);
	a[0] = sys_ptr(path);
	a[1] = (uint64_t)path_len;
	a[2] = sys_ptr(args);
	a[3] = args_len;
	a[4] = flags;
	a[5] = 0;
	return (v_syscall6(SYS_PROC_SPAWN, a));
}

int	v_proc_kill(t_handle proc, int code)
{
	return ((int)sys2(SYS_PROC_KILL, proc, (uint64_t)(int64_t)code));
}

int	v_proc_info(t_handle proc, t_procinfo *out)
{
	return ((int)sys3(SYS_PROC_INFO, proc, sys_ptr(out), sizeof(*out)));
}

int64_t	v_thread_create_raw(const t_vthreadreq *req)
{
	uint64_t	a[V_SYS_ARGS];

	if (!req)
		return (E_FAULT);
	a[0] = req->entry;
	a[1] = req->arg;
	a[2] = req->stack_size;
	a[3] = req->prio;
	a[4] = req->flags;
	a[5] = 0;
	return (v_syscall6(SYS_THREAD_CREATE, a));
}
