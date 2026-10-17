#include "help.h"

uint64_t	ha_glass_bytes(uint32_t width)
{
	uint64_t	cols;
	uint64_t	rows;

	cols = ((uint64_t)width + 3) / 4;
	rows = (80 + 3) / 4;
	return (2 * cols * rows * 4);
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
