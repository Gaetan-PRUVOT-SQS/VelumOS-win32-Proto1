#include <string.h>
#include "a02_fake.h"

t_bootinfo			g_fake_info;
static const char	*g_fault_mode;

const t_bootinfo	*boot_info(void)
{
	return (&g_fake_info);
}

t_bootinfo	*boot_info_rw(void)
{
	return (&g_fake_info);
}

int	boot_cmdline_has(const char *word)
{
	(void)word;
	return (0);
}

const char	*boot_cmdline_get(const char *key)
{
	if (g_fault_mode && !strcmp(key, "pmm_fault"))
		return (g_fault_mode);
	return (NULL);
}

void	fake_cmdline(const char *fault_mode)
{
	g_fault_mode = fault_mode;
}
