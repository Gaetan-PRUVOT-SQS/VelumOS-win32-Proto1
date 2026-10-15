#include <string.h>
#include "a02_fake.h"

int	model_allowed(uint64_t frame, uint64_t max)
{
	if (frame < 1 || frame >= g_model_span)
		return (0);
	if (frame < PMM_LOW_FRAMES && !(max && max <= PMM_LOW_END))
		return (0);
	if (max && (frame + 1) * PAGE_SIZE > max)
		return (0);
	return (1);
}

static int	run_free(uint64_t c, const t_mreq *rq)
{
	uint64_t	i;

	i = 0;
	while (i < rq->n)
	{
		if (g_model[c + i] != PMM_FREE || !model_allowed(c + i, rq->max))
			return (0);
		i++;
	}
	return (1);
}

int	model_feasible(const t_mreq *rq)
{
	uint64_t	al;
	uint64_t	c;

	al = rq->al;
	if (!al)
		al = 1;
	c = 0;
	while (c + rq->n <= g_model_span)
	{
		if (c % al == 0 && run_free(c, rq))
			return (1);
		c++;
	}
	return (0);
}

int	model_alloc_ok(const t_mreq *rq, uint64_t phys, t_pmm_owner o)
{
	uint64_t	al;

	al = rq->al;
	if (!al)
		al = 1;
	if (phys % PAGE_SIZE || (phys >> PAGE_SHIFT) % al)
		return (0);
	if (!run_free(phys >> PAGE_SHIFT, rq))
		return (0);
	memset(g_model + (phys >> PAGE_SHIFT), o, rq->n);
	return (1);
}

int	model_free(uint64_t phys, uint64_t n, t_pmm_owner o)
{
	uint64_t	f;
	uint64_t	i;

	f = phys >> PAGE_SHIFT;
	i = 0;
	while (i < n)
	{
		if (f + i >= g_model_span || g_model[f + i] != o)
			return (0);
		i++;
	}
	memset(g_model + f, PMM_FREE, n);
	return (1);
}
