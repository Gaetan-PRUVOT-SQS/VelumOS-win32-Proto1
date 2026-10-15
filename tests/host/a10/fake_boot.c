#include <string.h>
#include "fake.h"
#include "velum/boot.h"

static char	g_fboot_cmd[BOOT_CMDLINE_MAX];

void	fboot_set(const char *cmdline)
{
	strncpy(g_fboot_cmd, cmdline, sizeof(g_fboot_cmd) - 1);
}

int	boot_cmdline_has(const char *word)
{
	return (strstr(g_fboot_cmd, word) != NULL);
}
