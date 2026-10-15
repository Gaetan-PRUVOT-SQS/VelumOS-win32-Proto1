#include "time_int.h"
#include "weak_ext.h"
#include "velum/klog.h"

t_process	*a05_caller(void)
{
	if (!proc_current)
		return (NULL);
	return (proc_current());
}

static int64_t	sys_power(const t_sysargs *a)
{
	int	rc;

	rc = a05_check_priv(a05_caller(), PF_POWER);
	if (rc == 0)
		rc = a05_check_power(a->a[0]);
	if (rc < 0)
		return (rc);
	if (a->a[0] == POWER_OFF)
		return (power_off());
	return (power_reboot());
}

void	a05_syscalls_register(void)
{
	int	rc;

	if (!syscall_register)
	{
		klog_info("time: pas de table d'appels, 0x52-0x54 et 0x58 en attente");
		return ;
	}
	rc = syscall_register(SYS_TIME_MONO, sys_time_mono, "time_mono");
	if (rc >= 0)
		rc = syscall_register(SYS_TIME_WALL, sys_time_wall, "time_wall");
	if (rc >= 0)
		rc = syscall_register(SYS_TIME_SET_WALL, sys_time_set_wall,
				"time_set_wall");
	if (rc >= 0)
		rc = syscall_register(SYS_POWER, sys_power, "power");
	if (rc < 0)
		klog_err("time: enregistrement des appels système refusé (%d)", rc);
}
