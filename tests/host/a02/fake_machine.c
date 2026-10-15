#include <string.h>
#include "velum/err.h"
#include "a02_fake.h"

static uint64_t	pool_top(void)
{
	uint32_t	i;
	uint64_t	top;

	i = 0;
	top = 0;
	while (i < g_fake_info.nranges && i < BOOT_MAX_RANGES)
	{
		if (pmm_is_pool_type(g_fake_info.ranges[i].type))
			top = max_u64(top, pmm_range_end(&g_fake_info.ranges[i]));
		i++;
	}
	return (top);
}

void	fake_reset(void)
{
	fake_window_drop();
	fake_hook_clear();
	pmm_reset();
	memset(&g_fake_info, 0, sizeof(g_fake_info));
	g_fake_info.version = BOOT_INFO_VERSION;
	g_fake_info.size = sizeof(g_fake_info);
	fake_log_reset();
}

void	fake_range(uint64_t base, uint64_t length, uint32_t type)
{
	t_memrange	*r;

	if (g_fake_info.nranges >= BOOT_MAX_RANGES)
		return ;
	r = &g_fake_info.ranges[g_fake_info.nranges];
	r->base = base;
	r->length = length;
	r->type = type;
	r->reserved = 0;
	g_fake_info.nranges++;
}

int	fake_boot(void)
{
	uint64_t	top;

	top = pool_top();
	if (top > FAKE_WINDOW_MAX)
		top = PAGE_SIZE;
	if (fake_window_map(top) < 0)
		return (E_NOMEM);
	fake_window_poison();
	return (pmm_boot_init());
}
