#include <string.h>
#include "fake.h"
#include "velum/boot.h"
#include "velum/cpu.h"
#include "velum/err.h"
#include "velum/ksyscall.h"
#include "velum/timer.h"

uint32_t	cpu_count(void)
{
	return (g_fake.ncpus);
}

const t_cpufeat	*cpu_features(void)
{
	static t_cpufeat	f;

	memset(&f, 0, sizeof(f));
	memcpy(f.brand, g_fake.brand, sizeof(f.brand));
	return (&f);
}

const char	*boot_build_id(void)
{
	return ("0123456789abcdef0123456789abcdef01234567");
}

uint64_t	time_now_ns(void)
{
	return (g_fake.now_ns);
}

int	syscall_register(uint32_t num, t_sysfn fn, const char *name)
{
	(void)fn;
	if (g_fake.sys_rc < 0 || g_fake.nsys >= FAKE_SYSCALLS)
		return (g_fake.sys_rc);
	g_fake.sys[g_fake.nsys].num = num;
	g_fake.sys[g_fake.nsys].name = name;
	g_fake.nsys++;
	return (0);
}
