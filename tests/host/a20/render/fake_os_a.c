#include "velum/err.h"
#include "platform.h"
#include "render.h"

t_fakeos	g_fos;

uint64_t	os_mono_ns(void)
{
	return (g_fos.now);
}

uint64_t	os_wall_ns(void)
{
	return (WALL_NS + g_fos.now - FOS_T0);
}

int	os_sleep_ns(uint64_t ns)
{
	g_fos.now += ns;
	return (0);
}

int	os_timer_open(t_handle *out)
{
	*out = 7;
	return (0);
}

int	os_timer_after(t_handle timer, uint64_t delay_ns, uint64_t period_ns)
{
	(void)timer;
	(void)delay_ns;
	(void)period_ns;
	g_fos.timer_armed++;
	return (0);
}
