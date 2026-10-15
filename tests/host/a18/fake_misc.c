#include <stdlib.h>
#include <string.h>
#include "fake_kern.h"

void	fk_reset(void)
{
	int	i;

	i = 0;
	while (i < FK_OBJ_MAX)
	{
		g_fk.o[i].refs = 0;
		g_fk.o[i].maps = 0;
		fk_unref(i);
		i++;
	}
	free(g_fk.fb);
	memset(&g_fk, 0, sizeof(g_fk));
	g_fk.listener = -1;
	g_fk.fail_section = -1;
	g_fk.fail_map = -1;
	g_fk.fail_alloc = -1;
	g_fk.now = 1000000000ull;
}

void	fk_set_display(uint32_t w, uint32_t h)
{
	free(g_fk.fb);
	g_fk.fb_w = w;
	g_fk.fb_h = h;
	g_fk.fb_pitch = (w + 16) * 4;
	g_fk.fb = calloc(1, (size_t)g_fk.fb_pitch * h);
}

int	fk_fail(int64_t *counter)
{
	if (*counter < 0)
		return (0);
	if (*counter == 0)
	{
		*counter = -1;
		return (1);
	}
	(*counter)--;
	return (0);
}

uint32_t	fk_live(void)
{
	uint32_t	n;
	int			i;

	n = 0;
	i = 0;
	while (i < FK_OBJ_MAX)
	{
		if (g_fk.o[i].type != FK_FREE)
			n++;
		i++;
	}
	return (n);
}

int	fk_signaled(int obj)
{
	t_fkobj	*o;

	o = &g_fk.o[obj];
	if (o->type == FK_LISTEN)
		return (o->npend > 0);
	if (o->type == FK_CHAN)
		return (o->n > 0 || o->peer < 0);
	if (o->type == FK_INPUT)
		return (g_fk.in_n > 0);
	return (0);
}
