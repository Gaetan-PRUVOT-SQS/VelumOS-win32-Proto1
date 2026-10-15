#include "rng_int.h"
#include "velum/cpu.h"
#include "velum/err.h"
#include "velum/timer.h"

static uint64_t	plat_now_ns(void)
{
	return (time_now_ns());
}

static int	plat_rdseed(uint64_t *out)
{
	const t_cpufeat	*f;

	f = cpu_features();
	if (!f || !f->rdseed)
		return (E_NODEV);
	if (rng_x86_rdseed(out))
		return (0);
	return (E_AGAIN);
}

static int	plat_rdrand(uint64_t *out)
{
	const t_cpufeat	*f;

	f = cpu_features();
	if (!f || !f->rdrand)
		return (E_NODEV);
	if (rng_x86_rdrand(out))
		return (0);
	return (E_AGAIN);
}

const t_rng_env	g_rng_env = {plat_now_ns, rng_x86_rdtsc, plat_rdseed,
	plat_rdrand};
