#include <string.h>
#include "fake.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/timer.h"

int	wait_until(t_condfn cond, void *ctx, uint64_t timeout_ns)
{
	int	i;

	(void)timeout_ns;
	i = 0;
	while (i < FK_POLLS)
	{
		if (cond(ctx))
			return (0);
		fk_tick();
		i++;
	}
	g_fk.timeouts++;
	return (E_TIMEOUT);
}

int	boot_cmdline_has(const char *word)
{
	return (strstr(g_fk.cmdline, word) != NULL);
}

void	fk_tick(void)
{
	int	i;

	i = 0;
	while (i < FK_DEVS)
	{
		if (g_fkdev[i].present && g_fkdev[i].pending)
		{
			g_fkdev[i].pending = false;
			fk_process(&g_fkdev[i]);
		}
		i++;
	}
}
