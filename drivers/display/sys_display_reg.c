#include "display_int.h"
#include "velum/err.h"

int64_t	sys_display_map(const t_sysargs *a)
{
	t_process	*p;
	int64_t		rc;

	rc = display_priv();
	if (rc < 0)
		return (rc);
	if (!g_display.ready)
		return (E_NODEV);
	p = proc_current();
	mutex_lock(&g_display.lock);
	rc = display_user_map(p, a->a[0]);
	mutex_unlock(&g_display.lock);
	return (rc);
}

void	display_register_syscalls(void)
{
	syscall_register(SYS_DISPLAY_INFO, sys_display_info, "display_info");
	syscall_register(SYS_DISPLAY_MAP, sys_display_map, "display_map");
	syscall_register(SYS_DISPLAY_SET_MODE, sys_display_set_mode,
		"display_set_mode");
	syscall_register(SYS_DISPLAY_MODES, sys_display_modes, "display_modes");
	syscall_register(SYS_KCON, sys_kcon, "kcon");
}
