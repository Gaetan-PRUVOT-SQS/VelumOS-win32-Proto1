#include <string.h>
#include "fake.h"
#include "velum/err.h"

t_fsys	g_fsys;

void	fsys_reset(void)
{
	memset(&g_fsys, 0, sizeof(g_fsys));
}

int	syscall_register(uint32_t num, t_sysfn fn, const char *name)
{
	(void)name;
	if (num >= FSYS_MAX)
		return (E_INVAL);
	if (g_fsys.fail_next)
	{
		g_fsys.fail_next--;
		return (E_EXIST);
	}
	if (g_fsys.fn[num])
		return (E_EXIST);
	g_fsys.fn[num] = fn;
	return (0);
}

int64_t	fsys_call(uint32_t num, uint64_t a0, uint64_t a1, uint64_t a2)
{
	t_sysargs	args;

	memset(&args, 0, sizeof(args));
	args.a[0] = a0;
	args.a[1] = a1;
	args.a[2] = a2;
	if (num >= FSYS_MAX || !g_fsys.fn[num])
		return (E_NOSYS);
	return (g_fsys.fn[num](&args));
}
