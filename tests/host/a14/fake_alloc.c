#include "alloc_int.h"
#include "fake_alloc.h"

static int	g_hook_codes[FA_HOOK_MAX];
static int	g_hook_count;

static void	fa_hook(int code)
{
	if (g_hook_count < FA_HOOK_MAX)
		g_hook_codes[g_hook_count] = code;
	g_hook_count++;
}

void	fa_hook_install(void)
{
	alloc_set_fault_hook(fa_hook);
	g_hook_count = 0;
}

void	fa_hook_reset(void)
{
	g_hook_count = 0;
}

int	fa_hook_count(void)
{
	return (g_hook_count);
}

int	fa_hook_code(int i)
{
	if (i < 0 || i >= FA_HOOK_MAX)
		return (-1);
	return (g_hook_codes[i]);
}
