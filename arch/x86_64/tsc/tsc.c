#include "../../../kernel/time/time_int.h"
#include "velum/cpu.h"
#include "velum/irqflags.h"

#define CPUID_TSC_LEAF 0x15
#define CPUID_FREQ_LEAF 0x16
#define HZ_PER_MHZ 1000000ull
#define CAL_HPET_MS 20
#define CAL_SPIN_MAX 100000000ull

uint64_t	tsc_hz_cpuid(void)
{
	uint32_t	r[4];
	uint32_t	max;
	uint64_t	hz;

	cpu_cpuid(0, 0, r);
	max = r[0];
	if (max >= CPUID_TSC_LEAF)
	{
		cpu_cpuid(CPUID_TSC_LEAF, 0, r);
		hz = hz_from_cpuid15(r[0], r[1], r[2]);
		if (hz)
			return (hz);
	}
	if (max >= CPUID_FREQ_LEAF)
	{
		cpu_cpuid(CPUID_FREQ_LEAF, 0, r);
		return ((uint64_t)(r[0] & 0xffff) * HZ_PER_MHZ);
	}
	return (0);
}

uint64_t	tsc_calibrate_hpet(void)
{
	uint64_t	h0;
	uint64_t	h1;
	uint64_t	t0;
	uint64_t	target;
	uint64_t	spins;

	if (!hpet_present())
		return (0);
	h0 = hpet_read();
	t0 = tsc_read();
	target = h0 + hpet_hz() * CAL_HPET_MS / 1000;
	h1 = h0;
	spins = 0;
	while (h1 < target)
	{
		spins++;
		if (spins > CAL_SPIN_MAX)
			return (0);
		h1 = hpet_read();
	}
	return (hz_from_ref(tsc_read() - t0, h1 - h0, hpet_hz()));
}
