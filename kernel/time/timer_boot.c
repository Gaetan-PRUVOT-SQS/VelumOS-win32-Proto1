#include "time_int.h"
#include "velum/acpi.h"
#include "velum/cpu.h"
#include "velum/err.h"
#include "velum/klog.h"

#define KEEPALIVE_NS 60000000000ull

static uint64_t	tsc_choose_hz(void)
{
	uint64_t	cpuid_hz;
	uint64_t	meas;

	cpuid_hz = tsc_hz_cpuid();
	meas = tsc_calibrate_hpet();
	if (!meas)
		meas = tsc_calibrate_pit();
	if (meas < TSC_HZ_MIN || meas > TSC_HZ_MAX)
		meas = 0;
	if (cpuid_hz < TSC_HZ_MIN || cpuid_hz > TSC_HZ_MAX)
		cpuid_hz = 0;
	klog_info("time: TSC annoncé %llu Hz, mesuré %llu Hz", cpuid_hz, meas);
	if (cpuid_hz && (!meas || hz_coherent(cpuid_hz, meas, HZ_TOLERANCE_PPM)))
		return (cpuid_hz);
	if (cpuid_hz)
		klog_warn("time: fréquence CPUID incohérente, mesure retenue");
	return (meas);
}

static int	clock_boot(void)
{
	const t_cpufeat	*f;
	uint64_t		thz;
	uint64_t		hpet;

	f = cpu_features();
	hpet = acpi_info()->hpet_phys;
	if (hpet && hpet_init(hpet) < 0)
		klog_warn("time: HPET @%#llx inutilisable", hpet);
	thz = tsc_choose_hz();
	if (thz && (f->invariant_tsc || !hpet_present()))
	{
		if (!f->invariant_tsc)
			klog_warn("time: TSC non invariant et pas de HPET, TSC retenu");
		clock_install(CLOCK_SRC_TSC, thz, thz);
		return (E_OK);
	}
	if (!hpet_present())
		return (E_NODEV);
	clock_install(CLOCK_SRC_HPET, hpet_hz(), thz);
	return (E_OK);
}

static void	hpet_keepalive(void *ctx)
{
	(void)ctx;
	if (timer_arm(time_now_ns() + KEEPALIVE_NS, hpet_keepalive, NULL) < 0)
		klog_err("time: entretien du HPET 32 bits impossible");
}

static void	timer_log(void)
{
	const char	*src;
	const char	*mode;

	src = "TSC";
	if (clock_state()->source == CLOCK_SRC_HPET)
		src = "HPET";
	mode = "LAPIC one-shot";
	if (timer_mode() == TIMER_MODE_DEADLINE)
		mode = "TSC-deadline";
	klog_info("time: horloge %s à %llu Hz, minuterie %s, sans tic périodique",
		src, clock_state()->src_hz, mode);
}

int	timer_boot_init(void)
{
	int	rc;

	rc = clock_boot();
	if (rc < 0)
	{
		klog_err("time: aucune source de temps (ni TSC mesurable ni HPET)");
		return (rc);
	}
	wall_init();
	rc = timer_hw_init();
	if (rc < 0)
		return (rc);
	if (clock_state()->source == CLOCK_SRC_HPET && !hpet_is_wide())
		hpet_keepalive(NULL);
	a05_syscalls_register();
	timer_log();
	return (E_OK);
}
