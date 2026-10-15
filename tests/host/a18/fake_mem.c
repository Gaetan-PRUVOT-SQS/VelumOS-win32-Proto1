#include <string.h>
#include "fake_kern.h"

void	*fk_map(t_handle h, uint64_t len)
{
	int	obj;

	obj = fk_obj_of(h, FK_SECTION);
	if (obj < 0 || len == 0 || len > g_fk.o[obj].size)
		return (NULL);
	if (fk_fail(&g_fk.fail_map))
		return (NULL);
	g_fk.o[obj].maps++;
	return (g_fk.o[obj].mem);
}

void	fk_unmap(void *va)
{
	int	i;

	i = 0;
	while (i < FK_OBJ_MAX)
	{
		if (g_fk.o[i].type == FK_SECTION && g_fk.o[i].mem == va
			&& g_fk.o[i].maps > 0)
		{
			g_fk.o[i].maps--;
			if (g_fk.o[i].refs == 0 && g_fk.o[i].maps == 0)
				fk_unref(i);
			return ;
		}
		i++;
	}
}

void	fk_push_input(uint32_t type, uint32_t code, int32_t x, int32_t y)
{
	t_inpevent	*e;

	if (g_fk.in_n >= FK_INPUT_MAX)
		return ;
	e = &g_fk.in[(g_fk.in_head + g_fk.in_n) % FK_INPUT_MAX];
	memset(e, 0, sizeof(*e));
	e->type = type;
	e->code = code;
	e->x = x;
	e->y = y;
	e->time_ns = g_fk.now;
	g_fk.in_n++;
}

void	fk_push_key(uint32_t type, uint32_t code, uint32_t mods)
{
	t_inpevent	*e;

	if (g_fk.in_n >= FK_INPUT_MAX)
		return ;
	e = &g_fk.in[(g_fk.in_head + g_fk.in_n) % FK_INPUT_MAX];
	memset(e, 0, sizeof(*e));
	e->type = type;
	e->code = code;
	e->mods = mods;
	e->time_ns = g_fk.now;
	g_fk.in_n++;
}

uint32_t	fk_queued(t_handle ch)
{
	int	obj;

	obj = fk_obj_of(ch, FK_CHAN);
	if (obj < 0)
		return (0);
	return (g_fk.o[obj].n);
}
