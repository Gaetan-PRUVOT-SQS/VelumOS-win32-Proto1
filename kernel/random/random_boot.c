#include "rng_int.h"
#include "velum/klog.h"
#include "velum/random.h"

static const char	*yes_no(uint32_t src, uint32_t flag)
{
	if (src & flag)
		return ("oui");
	return ("non");
}

int	random_boot_init(void)
{
	uint32_t	src;
	int			rc;

	src = rng_reseed(rng_global());
	klog_info("random: graine rdseed=%s rdrand=%s gigue=%s horloge=%s",
		yes_no(src, RNG_SRC_RDSEED), yes_no(src, RNG_SRC_RDRAND),
		yes_no(src, RNG_SRC_JITTER), yes_no(src, RNG_SRC_TIME));
	if (!(src & (RNG_SRC_RDSEED | RNG_SRC_RDRAND)))
		klog_warn("random: ni RDSEED ni RDRAND, graine par gigue seule");
	rc = syscall_register(SYS_GETRANDOM, sys_getrandom, "getrandom");
	if (rc < 0)
		return (rc);
	rc = syscall_register(SYS_SYSINFO, sys_sysinfo, "sysinfo");
	if (rc < 0)
		return (rc);
	return (0);
}
