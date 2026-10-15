#include "proc_int.h"
#include "proc_sys.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"

static const t_sysdef	g_sysdefs[] = {
{SYS_EXIT, sys_exit, "exit"}, {SYS_PROC_SPAWN, sys_proc_spawn, "proc_spawn"},
{SYS_PROC_KILL, sys_proc_kill, "proc_kill"},
{SYS_PROC_INFO, sys_proc_info, "proc_info"},
{SYS_THREAD_CREATE, sys_thread_create, "thread_create"},
{SYS_THREAD_EXIT, sys_thread_exit, "thread_exit"},
{SYS_THREAD_PRIO, sys_thread_prio, "thread_prio"},
{SYS_GETTID, sys_gettid, "gettid"},
{SYS_SET_FSBASE, sys_set_fsbase, "set_fsbase"},
{SYS_PROC_LIST, sys_proc_list, "proc_list"}, {SYS_LOG, sys_log, "log"},
{0, NULL, NULL}
};

int	proc_sys_register(void)
{
	uint32_t	i;
	int			rc;

	i = 0;
	while (g_sysdefs[i].fn)
	{
		rc = syscall_register(g_sysdefs[i].num, g_sysdefs[i].fn,
				g_sysdefs[i].name);
		if (rc < 0)
			return (rc);
		i++;
	}
	return (0);
}

int	syscall_boot_init(void)
{
	int	rc;

	rc = syscall_arch_init();
	if (rc == 0)
		rc = proc_sys_register();
	if (rc == 0)
		klog_info("syscall: entrée prête, appels 0x00 à 0x0a enregistrés");
	return (rc);
}

int	syscall_require(uint32_t pf)
{
	t_process	*p;

	p = proc_current();
	if (!p || (p->flags & pf) != pf)
		return (E_PERM);
	return (0);
}

int	syscall_selftest(void)
{
	t_sysargs	args;
	int			rc;

	rc = syscall_arch_selftest();
	if (rc != 0)
		return (rc);
	memset(&args, 0, sizeof(args));
	if (syscall_dispatch(&args, SYS_MAX) != E_NOSYS)
		return (10);
	if (syscall_dispatch(&args, 0x10000) != E_NOSYS)
		return (11);
	if (syscall_register(SYS_EXIT, sys_gettid, "double") != E_EXIST)
		return (12);
	if (syscall_register(SYS_MAX, sys_gettid, "hors table") != E_INVAL)
		return (13);
	if (syscall_calls(SYS_MAX) != 0)
		return (14);
	return (0);
}
