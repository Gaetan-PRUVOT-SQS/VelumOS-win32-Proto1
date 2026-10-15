#include "velum/err.h"
#include "velum/libk.h"
#include "platform.h"
#include "render.h"

int	os_wait_exit(t_handle proc)
{
	(void)proc;
	return (0);
}

void	os_kill(t_handle proc)
{
	(void)proc;
	g_fos.killed++;
}

void	os_close(t_handle h)
{
	(void)h;
}

int	os_power(uint32_t op)
{
	g_fos.power_op = (int)op;
	return (0);
}

int	os_sysinfo(t_sysinfo *out)
{
	memset(out, 0, sizeof(*out));
	out->uptime_ns = 3723ull * 1000000000ull + g_fos.now;
	return (0);
}
