#include "fake.h"

int	fake_mem_live(void)
{
	return (g_fake_mem.live);
}

int	fake_mem_calls(void)
{
	return (g_fake_mem.calls);
}

int	fake_mem_failed(void)
{
	return (g_fake_mem.failed);
}

void	fake_mem_arm(int fail_at)
{
	g_fake_mem.calls = 0;
	g_fake_mem.fail_at = fail_at;
	g_fake_mem.failed = 0;
}
