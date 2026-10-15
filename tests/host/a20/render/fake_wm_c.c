#include "velum/err.h"
#include "velum/libk.h"
#include "render.h"

void	wmc_disconnect(void)
{
	g_fwm.count = 0;
}

int	wmc_next(void *buf, uint32_t size, uint64_t timeout_ns)
{
	(void)buf;
	(void)size;
	(void)timeout_ns;
	return (E_TIMEOUT);
}

t_handle	wmc_channel(void)
{
	return (1);
}

t_fwin	*fwm_by_id(uint32_t id)
{
	uint32_t	i;

	i = 0;
	while (i < g_fwm.count)
	{
		if (g_fwm.win[i].id == id)
			return (&g_fwm.win[i]);
		i++;
	}
	return (NULL);
}

t_fwin	*fwm_find(const char *title)
{
	uint32_t	i;

	i = 0;
	while (i < g_fwm.count)
	{
		if (g_fwm.win[i].id && !strcmp(g_fwm.win[i].title, title))
			return (&g_fwm.win[i]);
		i++;
	}
	return (NULL);
}
