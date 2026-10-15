#include "fakes.h"
#include "velum/err.h"
#include "velum/vmm.h"

int	copy_to_user(t_uptr dst, const void *src, size_t n)
{
	if (dst == 0 || dst == FAKE_BAD_PTR || dst < g_fp.ok_base)
		return (E_FAULT);
	if (n > g_fp.ok_len || dst - g_fp.ok_base > g_fp.ok_len - n)
		return (E_FAULT);
	memcpy((void *)(uintptr_t)dst, src, n);
	return (E_OK);
}

int	syscall_register(uint32_t num, t_sysfn fn, const char *name)
{
	(void)name;
	if (num >= 256)
		return (E_INVAL);
	if (g_fp.sys[num])
		return (E_EXIST);
	g_fp.sys[num] = fn;
	return (E_OK);
}

int64_t	fake_sys(uint32_t num, uint64_t a0, uint64_t a1, uint64_t a2)
{
	t_sysargs	args;

	memset(&args, 0, sizeof(args));
	args.a[0] = a0;
	args.a[1] = a1;
	args.a[2] = a2;
	if (num >= 256 || !g_fp.sys[num])
		return (E_NOSYS);
	return (g_fp.sys[num](&args));
}
