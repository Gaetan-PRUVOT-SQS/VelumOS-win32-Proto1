#include <stdlib.h>
#include "fake.h"

t_fakemem	g_fake_mem;

void	fake_mem_reset(int fail_at)
{
	g_fake_mem.live = 0;
	g_fake_mem.calls = 0;
	g_fake_mem.fail_at = fail_at;
	g_fake_mem.failed = 0;
}

void	*ctl_alloc(size_t size)
{
	void	*p;

	g_fake_mem.calls++;
	if (g_fake_mem.fail_at > 0 && g_fake_mem.calls == g_fake_mem.fail_at)
	{
		g_fake_mem.failed++;
		return (NULL);
	}
	p = calloc(1, size + 1);
	if (p)
		g_fake_mem.live++;
	return (p);
}

void	ctl_free(void *ptr)
{
	if (!ptr)
		return ;
	g_fake_mem.live--;
	free(ptr);
}
