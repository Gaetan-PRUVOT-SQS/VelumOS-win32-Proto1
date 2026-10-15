#include "layout.h"
#include "velum/libk.h"

static const t_layout *const	g_layouts[] = {&g_layout_fr, &g_layout_us, 0};

const t_layout	*layout_find(const char *name)
{
	uint32_t	i;

	if (!name)
		return (0);
	i = 0;
	while (g_layouts[i])
	{
		if (strcmp(g_layouts[i]->name, name) == 0)
			return (g_layouts[i]);
		i++;
	}
	return (0);
}

const t_layout	*layout_default(void)
{
	return (&g_layout_fr);
}

const t_key	*layout_key(const t_layout *l, uint16_t code)
{
	uint32_t	i;

	i = 0;
	while (i < l->nkeys)
	{
		if (l->keys[i].code == code)
			return (&l->keys[i]);
		i++;
	}
	return (common_key(code));
}
