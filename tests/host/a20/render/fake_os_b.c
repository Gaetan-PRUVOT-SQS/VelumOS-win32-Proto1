#include "velum/err.h"
#include "velum/libk.h"
#include "platform.h"
#include "render.h"

int	os_timer_cancel(t_handle timer)
{
	(void)timer;
	g_fos.timer_armed = 0;
	return (0);
}

int	os_wait1(t_handle h, uint64_t timeout_ns)
{
	(void)h;
	(void)timeout_ns;
	return (E_TIMEOUT);
}

int	os_wait2(t_handle a, t_handle b, uint64_t timeout_ns)
{
	(void)a;
	(void)b;
	(void)timeout_ns;
	return (E_TIMEOUT);
}

int	os_spawn(const char *path, const char *arg, uint32_t flags, t_handle *proc)
{
	uint32_t	n;

	n = g_fos.nspawn;
	if (n >= FOS_SPAWN_MAX)
		return (E_NOMEM);
	strlcpy(g_fos.path[n], path, FOS_TEXT_MAX);
	g_fos.arg[n][0] = '\0';
	if (arg)
		strlcpy(g_fos.arg[n], arg, FOS_TEXT_MAX);
	g_fos.flags[n] = flags;
	g_fos.nspawn++;
	*proc = 100 + n;
	return (0);
}

int	os_exited(t_handle proc)
{
	(void)proc;
	return (0);
}
