#include "velum/panic.h"
#include "stackguard_seed.h"

#define CPUID_LEAF1_ECX_RDRAND (1u << 30)
#define CPUID_LEAF7_EBX_RDSEED (1u << 18)

static t_guardsrc	g_guard_src;

static void	cpuid_leaf(uint32_t leaf, uint32_t sub, uint32_t *r)
{
	__asm__ volatile ("cpuid"
		: "=a"(r[0]), "=b"(r[1]), "=c"(r[2]), "=d"(r[3])
		: "a"(leaf), "c"(sub));
}

static bool	hw_rdrand(uint64_t *out)
{
	uint64_t		v;
	unsigned char	ok;

	__asm__ volatile ("rdrand %0; setc %1" : "=r"(v), "=qm"(ok) : : "cc");
	*out = v;
	return (ok != 0);
}

static bool	hw_rdseed(uint64_t *out)
{
	uint64_t		v;
	unsigned char	ok;

	__asm__ volatile ("rdseed %0; setc %1" : "=r"(v), "=qm"(ok) : : "cc");
	*out = v;
	if (!ok)
		__asm__ volatile ("pause");
	return (ok != 0);
}

static uint64_t	hw_tsc(void)
{
	uint32_t	lo;
	uint32_t	hi;

	__asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
	return (((uint64_t)hi << 32) | lo);
}

__attribute__((no_stack_protector, noinline))
uint64_t	boot_guard_seed(void)
{
	t_guardhw	hw;
	uint32_t	r[4];
	uint32_t	max_leaf;

	cpuid_leaf(0, 0, r);
	max_leaf = r[0];
	hw.has_rdrand = false;
	hw.has_rdseed = false;
	if (max_leaf >= 1)
	{
		cpuid_leaf(1, 0, r);
		hw.has_rdrand = (r[2] & CPUID_LEAF1_ECX_RDRAND) != 0;
	}
	if (max_leaf >= 7)
	{
		cpuid_leaf(7, 0, r);
		hw.has_rdseed = (r[1] & CPUID_LEAF7_EBX_RDSEED) != 0;
	}
	hw.rdrand = hw_rdrand;
	hw.rdseed = hw_rdseed;
	hw.tsc = hw_tsc;
	return (guard_seed(&hw, &g_guard_src));
}

const char	*boot_guard_source(void)
{
	if (g_guard_src == GUARD_SRC_BOTH)
		return ("rdseed+rdrand");
	if (g_guard_src == GUARD_SRC_RDRAND)
		return ("rdrand");
	if (g_guard_src == GUARD_SRC_RDSEED)
		return ("rdseed");
	return ("tsc");
}

_Noreturn void	__stack_chk_fail(void)
{
	panic("stack smashing detected (from %p)", __builtin_return_address(0));
}
