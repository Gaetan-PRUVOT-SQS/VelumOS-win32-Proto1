#include <string.h>
#include "fake.h"
#include "velum/err.h"
#include "velum/ksyscall.h"
#include "velum/vmm.h"

bool	user_range_ok(t_uptr addr, size_t n)
{
	if (addr < 4096 || addr + n < addr)
		return (false);
	return (addr + n <= 0x0000800000000000ull);
}

int	copy_from_user(void *dst, t_uptr src, size_t n)
{
	if (!user_range_ok(src, n))
		return (E_FAULT);
	memcpy(dst, (const void *)src, n);
	return (0);
}

int	copy_to_user(t_uptr dst, const void *src, size_t n)
{
	if (!user_range_ok(dst, n))
		return (E_FAULT);
	memcpy((void *)dst, src, n);
	return (0);
}

int	syscall_register(uint32_t num, t_sysfn fn, const char *name)
{
	(void)name;
	if (num >= SYS_MAX || fn == NULL)
		return (E_INVAL);
	return (0);
}

uint32_t	fake_rand(uint64_t *s)
{
	*s ^= *s >> 12;
	*s ^= *s << 25;
	*s ^= *s >> 27;
	return ((uint32_t)((*s * 0x2545F4914F6CDD1Dull) >> 32));
}
