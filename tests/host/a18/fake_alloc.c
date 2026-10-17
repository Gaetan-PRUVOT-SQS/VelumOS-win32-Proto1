#include <stdlib.h>
#include <string.h>
#include "fake_kern.h"
#include "ws_sys.h"

t_fkmem	g_fkmem;

void	fk_alloc_reset(void)
{
	memset(&g_fkmem, 0, sizeof(g_fkmem));
}

static int	slot_of(const void *p)
{
	int	i;

	i = 0;
	while (i < FK_ALLOC_MAX)
	{
		if (g_fkmem.p[i] == p)
			return (i);
		i++;
	}
	return (-1);
}

void	*ws_sys_alloc(uint64_t size)
{
	int		i;
	void	*p;

	g_fkmem.calls++;
	if (size == 0 || fk_fail(&g_fk.fail_alloc))
		return (NULL);
	i = slot_of(NULL);
	if (i < 0)
		return (NULL);
	p = calloc(1, size);
	if (p == NULL)
		return (NULL);
	g_fkmem.p[i] = p;
	g_fkmem.size[i] = size;
	g_fkmem.live++;
	return (p);
}

void	ws_sys_free(void *p, uint64_t size)
{
	int	i;

	if (p == NULL)
		return ;
	i = slot_of(p);
	if (i < 0)
	{
		g_fkmem.bad++;
		return ;
	}
	if (g_fkmem.size[i] != size)
		g_fkmem.bad++;
	free(p);
	g_fkmem.p[i] = NULL;
	g_fkmem.size[i] = 0;
	g_fkmem.live--;
}
