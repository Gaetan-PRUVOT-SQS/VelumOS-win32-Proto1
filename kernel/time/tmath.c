#include "time_int.h"
#include "velum/err.h"

int	clockconv_init(t_clockconv *c, uint64_t num, uint64_t den)
{
	t_u128	m;

	if (!den || !num)
		return (E_INVAL);
	m = ((t_u128)num << CONV_SHIFT) / den;
	if (m == 0 || (m >> 64))
		return (E_RANGE);
	c->mult = (uint64_t)m;
	c->shift = CONV_SHIFT;
	c->reserved = 0;
	return (E_OK);
}

uint64_t	clockconv_apply(const t_clockconv *c, uint64_t v)
{
	t_u128	r;

	r = ((t_u128)v * c->mult) >> c->shift;
	if (r >> 64)
		return (UINT64_MAX);
	return ((uint64_t)r);
}

uint64_t	hz_from_ref(uint64_t dcycles, uint64_t dref, uint64_t ref_hz)
{
	t_u128	r;

	if (!dref)
		return (0);
	r = (t_u128)dcycles * ref_hz / dref;
	if (r >> 64)
		return (UINT64_MAX);
	return ((uint64_t)r);
}

bool	hz_coherent(uint64_t a, uint64_t b, uint32_t ppm)
{
	uint64_t	diff;

	if (!a || !b)
		return (false);
	diff = a - b;
	if (b > a)
		diff = b - a;
	return ((t_u128)diff * 1000000u <= (t_u128)b * ppm);
}

uint64_t	hz_from_cpuid15(uint32_t eax, uint32_t ebx, uint32_t ecx)
{
	if (!eax || !ebx || !ecx)
		return (0);
	return ((uint64_t)ecx * ebx / eax);
}
