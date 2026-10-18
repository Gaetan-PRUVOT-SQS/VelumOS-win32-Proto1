#include <stdio.h>
#include <string.h>
#include "../harness.h"
#include "velum/abi/abi_syscall.h"

#define WRAPPER_PATH "user/libvelum/sys_meminfo.c"

_Static_assert(SYS_MEMINFO == 0xbc, "SYS_MEMINFO");

static int	fichier_contient(const char *path, const char *needle)
{
	FILE	*f;
	char	line[256];
	int		trouve;

	f = fopen(path, "r");
	if (!f)
		return (0);
	trouve = 0;
	while (fgets(line, (int)sizeof(line), f))
	{
		if (strstr(line, needle))
			trouve = 1;
	}
	fclose(f);
	return (trouve);
}

static void	numero_public(void)
{
	h_eq_u64("SYS_MEMINFO", SYS_MEMINFO, 0xbc);
}

static void	emballage_un_argument(void)
{
	h_true(fichier_contient(WRAPPER_PATH, "sys1(SYS_MEMINFO,"),
		"emballage sys1(SYS_MEMINFO)");
	h_true(!fichier_contient(WRAPPER_PATH, "sys0(SYS_MEMINFO")
		&& !fichier_contient(WRAPPER_PATH, "sys2(SYS_MEMINFO")
		&& !fichier_contient(WRAPPER_PATH, "sys3(SYS_MEMINFO")
		&& !fichier_contient(WRAPPER_PATH, "sys4(SYS_MEMINFO")
		&& !fichier_contient(WRAPPER_PATH, "sys5(SYS_MEMINFO")
		&& !fichier_contient(WRAPPER_PATH, "sys6(SYS_MEMINFO")
		&& !fichier_contient(WRAPPER_PATH, "v_syscall6(SYS_MEMINFO"),
		"un argument, pas un autre emballage");
}

int	main(void)
{
	h_begin("e10/meminfo emballage");
	h_run("numéro public 0xbc", numero_public);
	h_run("emballage libc à un argument", emballage_un_argument);
	return (h_end());
}
