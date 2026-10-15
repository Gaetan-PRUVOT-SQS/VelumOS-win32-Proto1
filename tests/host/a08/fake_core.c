#include <stdlib.h>
#include <string.h>
#include "fake.h"

t_fake	g_fk;

void	fk_reset(void)
{
	memset(&g_fk, 0, sizeof(g_fk));
	g_fk.wait_end_rc = E_CANCELED;
	g_fk.next_va = FK_VA_BASE;
	g_fk.next_tid = 1;
	g_fk.now = 1000;
	if (object_boot_init() < 0)
		abort();
}

int	syscall_register(uint32_t num, t_sysfn fn, const char *name)
{
	(void)name;
	if (num >= FK_SYS)
		return (E_INVAL);
	if (g_fk.sys[num])
		return (E_EXIST);
	g_fk.sys[num] = fn;
	return (0);
}

int64_t	fk_call6(uint32_t num, const uint64_t *args)
{
	t_sysargs	sa;

	memset(&sa, 0, sizeof(sa));
	memcpy(sa.a, args, sizeof(sa.a));
	if (num >= FK_SYS || !g_fk.sys[num])
		return (E_NOSYS);
	return (g_fk.sys[num](&sa));
}

int64_t	fk_call(uint32_t num, uint64_t a0, uint64_t a1, uint64_t a2)
{
	uint64_t	args[6];

	memset(args, 0, sizeof(args));
	args[0] = a0;
	args[1] = a1;
	args[2] = a2;
	return (fk_call6(num, args));
}
