#include "sched_int.h"
#include "velum/klog.h"
#include "velum/ksyscall.h"

extern int	syscall_register(uint32_t num, t_sysfn fn,
				const char *name) __attribute__((weak));

static int64_t	sys_yield(const t_sysargs *args)
{
	(void)args;
	sched_yield();
	return (0);
}

static int64_t	sys_sleep(const t_sysargs *args)
{
	sched_sleep_ns(args->a[0]);
	return (0);
}

void	sched_syscalls_register(void)
{
	int	rc;

	if (!syscall_register)
	{
		klog_warn("sched: table d'appels absente, YIELD et SLEEP non "
			"enregistrés");
		return ;
	}
	rc = syscall_register(SYS_YIELD, sys_yield, "yield");
	if (rc < 0)
		klog_warn("sched: enregistrement de SYS_YIELD refusé (%d)", rc);
	rc = syscall_register(SYS_SLEEP, sys_sleep, "sleep");
	if (rc < 0)
		klog_warn("sched: enregistrement de SYS_SLEEP refusé (%d)", rc);
}
