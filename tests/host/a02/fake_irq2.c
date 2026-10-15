#include <string.h>
#include "a02_fake.h"

void	fake_hook_arm(int at_call, void (*fn)(void))
{
	g_fakeirq.calls = 0;
	g_fakeirq.hook_at = at_call;
	g_fakeirq.hook_fired = 0;
	g_fakeirq.hook = fn;
}

int	fake_hook_fired(void)
{
	return (g_fakeirq.hook_fired);
}

void	fake_hook_clear(void)
{
	memset(&g_fakeirq, 0, sizeof(g_fakeirq));
}

void	fake_relax_arm(void)
{
	g_fakeirq.relax_calls = 0;
	g_fakeirq.relax_release = 1;
}

int	fake_relax_calls(void)
{
	return (g_fakeirq.relax_calls);
}
