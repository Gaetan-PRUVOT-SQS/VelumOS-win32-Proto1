#include "luna_int.h"

static uint32_t	*g_scratch;
static size_t	g_scratch_px;

void	luna_glass_scratch(uint32_t *buf, size_t px)
{
	g_scratch = buf;
	g_scratch_px = px;
	if (buf == NULL)
		g_scratch_px = 0;
}

bool	lp_gscratch(size_t px, uint32_t **a, uint32_t **b)
{
	if (g_scratch == NULL || px == 0 || px > g_scratch_px / 2)
		return (false);
	*a = g_scratch;
	*b = g_scratch + px;
	return (true);
}
