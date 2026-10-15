#include <stdlib.h>
#include <string.h>
#include "a02_fake.h"

void	fake_snap_take(t_snap *s)
{
	s->bits = malloc(g_pmm.words * sizeof(uint64_t));
	s->owner = malloc(g_pmm.span);
	memcpy(s->bits, g_pmm.bits, g_pmm.words * sizeof(uint64_t));
	memcpy(s->owner, g_pmm.owner, g_pmm.span);
	pmm_get_stats(&s->stats);
}

int	fake_snap_same(const t_snap *s)
{
	t_pmm_stats	now;

	pmm_get_stats(&now);
	now.alloc_calls = s->stats.alloc_calls;
	now.free_calls = s->stats.free_calls;
	now.fail_calls = s->stats.fail_calls;
	if (memcmp(s->bits, g_pmm.bits, g_pmm.words * sizeof(uint64_t)))
		return (0);
	if (memcmp(s->owner, g_pmm.owner, g_pmm.span))
		return (0);
	return (!memcmp(&now, &s->stats, sizeof(now)));
}

void	fake_snap_drop(t_snap *s)
{
	free(s->bits);
	free(s->owner);
	s->bits = NULL;
	s->owner = NULL;
}
