#include "help.h"
#include "velum/luna_launcher.h"

uint64_t	ha_glass_bytes(uint32_t width)
{
	uint64_t	cols;
	uint64_t	rows;
	uint64_t	bar;
	uint64_t	panel;

	cols = ((uint64_t)width + 3) / 4;
	rows = (80 + 3) / 4;
	bar = 2 * cols * rows * 4;
	if (width < LUNA_LAUNCHER_WIDTH)
		return (bar);
	panel = 2 * (((uint64_t)LUNA_LAUNCHER_WIDTH + 3) / 4)
		* (((uint64_t)LUNA_LAUNCHER_HEIGHT + 3) / 4) * 4;
	if (panel > bar)
		return (panel);
	return (bar);
}

bool	ha_has_size(uint64_t size)
{
	uint32_t	i;

	i = 0;
	while (i < FK_ALLOC_MAX)
	{
		if (g_fkmem.p[i] != NULL && g_fkmem.size[i] == size)
			return (true);
		i++;
	}
	return (false);
}

int	ha_start(t_wsrv *s, int64_t fail, uint32_t w, uint32_t h)
{
	fk_alloc_reset();
	fk_reset();
	fk_set_display(w, h);
	g_fk.fail_alloc = fail;
	return (ws_open(s));
}
