#include "luna_int.h"

static const t_lunametrics	g_metrics = {50, 0, 0, 28, 24, 80, 42, 190, 17,
	36, 24, 56};
static const t_lunadetail	g_detail = {2, 13, 13, 13, 13, 40, 6, 24, 36, 24,
	6, 10, 13, 6, 8, 8, 2, 54, 38, 150, 24, 48, 3, 8, 24, 6, 14, 22};

void	luna_metrics(t_lunametrics *out)
{
	if (out)
		*out = g_metrics;
}

t_cmet	lm_metrics(void)
{
	return (&g_metrics);
}

t_cdet	lm_detail(void)
{
	return (&g_detail);
}
