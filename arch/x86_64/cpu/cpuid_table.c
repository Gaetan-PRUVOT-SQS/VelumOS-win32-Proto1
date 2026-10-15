#include "velum/libk.h"
#include "cpu_int.h"

static const t_featbit	g_featbits[FEAT_COUNT + 1] = {
{"nx", offsetof(t_cpufeat, nx), CPUID_E1, R_EDX, 20},
{"smep", offsetof(t_cpufeat, smep), CPUID_L7, R_EBX, 7},
{"smap", offsetof(t_cpufeat, smap), CPUID_L7, R_EBX, 20},
{"umip", offsetof(t_cpufeat, umip), CPUID_L7, R_ECX, 2},
{"pcid", offsetof(t_cpufeat, pcid), CPUID_L1, R_ECX, 17},
{"fsgsbase", offsetof(t_cpufeat, fsgsbase), CPUID_L7, R_EBX, 0},
{"xsave", offsetof(t_cpufeat, xsave), CPUID_L1, R_ECX, 26},
{"rdrand", offsetof(t_cpufeat, rdrand), CPUID_L1, R_ECX, 30},
{"rdseed", offsetof(t_cpufeat, rdseed), CPUID_L7, R_EBX, 18},
{"x2apic", offsetof(t_cpufeat, x2apic), CPUID_L1, R_ECX, 21},
{"tscdl", offsetof(t_cpufeat, tsc_deadline), CPUID_L1, R_ECX, 24},
{"invtsc", offsetof(t_cpufeat, invariant_tsc), CPUID_E7, R_EDX, 8},
{"pat", offsetof(t_cpufeat, pat), CPUID_L1, R_EDX, 16},
{"pge", offsetof(t_cpufeat, pge), CPUID_L1, R_EDX, 13},
{"sse2", offsetof(t_cpufeat, sse2), CPUID_L1, R_EDX, 26},
{"hv", offsetof(t_cpufeat, hypervisor), CPUID_L1, R_ECX, 31},
{NULL, 0, 0, 0, 0}
};

const t_featbit	*cpuid_featbits(void)
{
	return (g_featbits);
}

void	cpufeat_flags(const t_cpufeat *feat, char *buf, size_t size)
{
	const t_featbit	*fb;

	if (!size)
		return ;
	buf[0] = '\0';
	fb = g_featbits;
	while (fb->name)
	{
		if (*(const bool *)((const char *)feat + fb->offset))
		{
			if (buf[0])
				strlcat(buf, " ", size);
			strlcat(buf, fb->name, size);
		}
		fb++;
	}
}
