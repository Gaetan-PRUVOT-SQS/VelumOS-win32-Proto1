#include <stdlib.h>
#include <string.h>
#include "a02_fake.h"

uint8_t		*g_model;
uint64_t	g_model_span;

static void	model_mark_range(const t_memrange *r)
{
	uint64_t	lo;
	uint64_t	hi;

	lo = (r->base + PAGE_SIZE - 1) / PAGE_SIZE;
	hi = (r->base + r->length) / PAGE_SIZE;
	if (lo < 1)
		lo = 1;
	if (hi > lo)
		memset(g_model + lo, PMM_FREE, hi - lo);
}

int	model_init(void)
{
	uint32_t	i;

	free(g_model);
	g_model_span = g_pmm.span;
	g_model = malloc(g_model_span);
	if (!g_model)
		return (-1);
	memset(g_model, PMM_MARK_RESERVED, g_model_span);
	i = 0;
	while (i < g_fake_info.nranges)
	{
		if (g_fake_info.ranges[i].type == MEM_USABLE)
			model_mark_range(&g_fake_info.ranges[i]);
		i++;
	}
	memset(g_model + (g_pmm.meta_phys >> PAGE_SHIFT), PMM_MARK_META,
		g_pmm.meta_pages);
	return (0);
}

void	model_done(void)
{
	free(g_model);
	g_model = NULL;
	g_model_span = 0;
}

uint64_t	model_count(int mark)
{
	uint64_t	f;
	uint64_t	n;

	f = 0;
	n = 0;
	while (f < g_model_span)
	{
		n += (g_model[f] == mark);
		f++;
	}
	return (n);
}

uint64_t	model_diff(void)
{
	uint64_t	f;
	uint64_t	bad;

	f = 0;
	bad = 0;
	while (f < g_model_span)
	{
		bad += (g_model[f] != g_pmm.owner[f]);
		f++;
	}
	return (bad);
}
