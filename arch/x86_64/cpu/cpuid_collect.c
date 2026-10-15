#include "velum/libk.h"
#include "cpu_int.h"

static const uint32_t	g_leaves[CPUID_NLEAVES] = {
	0x0, 0x1, 0x7, 0xd, 0x80000000, 0x80000001, 0x80000002, 0x80000003,
	0x80000004, 0x80000007, 0x80000008
};

static bool	leaf_ok(uint32_t leaf, uint32_t max_basic, uint32_t max_ext)
{
	if (leaf < 0x80000000)
		return (leaf <= max_basic);
	return (max_ext != 0 && leaf <= max_ext);
}

static uint32_t	ext_max(t_cpuid_raw *raw)
{
	uint32_t	max;

	max = raw->r[CPUID_E0][R_EAX];
	if (max < 0x80000000 || max > 0x8000ffff)
	{
		memset(raw->r[CPUID_E0], 0, sizeof(raw->r[CPUID_E0]));
		return (0);
	}
	return (max);
}

void	cpuid_collect(t_cpuid_raw *raw, t_cpuidfn fn)
{
	uint32_t	max_basic;
	uint32_t	max_ext;
	int			i;

	memset(raw, 0, sizeof(*raw));
	fn(0, 0, raw->r[CPUID_L0]);
	max_basic = raw->r[CPUID_L0][R_EAX];
	fn(0x80000000, 0, raw->r[CPUID_E0]);
	max_ext = ext_max(raw);
	i = CPUID_L1;
	while (i < CPUID_NLEAVES)
	{
		if (i != CPUID_E0 && leaf_ok(g_leaves[i], max_basic, max_ext))
			fn(g_leaves[i], 0, raw->r[i]);
		i++;
	}
}

bool	cpuid_bit(const t_cpuid_raw *raw, int leaf, int reg, int bit)
{
	if (leaf < 0 || leaf >= CPUID_NLEAVES || reg < 0 || reg > R_EDX
		|| bit < 0 || bit > 31)
		return (false);
	return ((raw->r[leaf][reg] >> bit) & 1u);
}
