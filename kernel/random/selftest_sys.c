#include "rng_int.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/irqflags.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/proc.h"

static int	getrandom_checks(void)
{
	t_sysargs	a;
	uint8_t		buf[16];

	memset(&a, 0, sizeof(a));
	a.a[0] = (uint64_t)(uintptr_t)buf;
	a.a[1] = sizeof(buf);
	if (sys_getrandom(&a) != E_FAULT)
		return (1);
	a.a[1] = RANDOM_SYS_MAX + 1;
	if (sys_getrandom(&a) != E_INVAL)
		return (2);
	a.a[1] = sizeof(buf);
	a.a[2] = 1;
	if (sys_getrandom(&a) != E_INVAL)
		return (3);
	a.a[1] = 0;
	a.a[2] = 0;
	if (sys_getrandom(&a) != 0)
		return (4);
	return (0);
}

static int	nprocs_without_heap(void)
{
	t_sysinfo	si;
	uint64_t	irq;
	uint32_t	n;

	irq = irq_save();
	heap_fail_after(0);
	sysinfo_collect(&si);
	heap_fail_after(-1);
	n = (uint32_t)proc_count();
	irq_restore(irq);
	return (si.nprocs == n);
}

static int	sysinfo_checks(void)
{
	t_sysargs	a;
	t_sysinfo	si;
	uint8_t		buf[16];

	memset(&a, 0, sizeof(a));
	a.a[0] = (uint64_t)(uintptr_t)buf;
	if (sys_sysinfo(&a) != E_FAULT)
		return (5);
	sysinfo_collect(&si);
	if (si.abi_version != VELUM_ABI_VERSION || si.ncpus < 1)
		return (6);
	if (si.mem_total == 0 || si.mem_free > si.mem_total)
		return (7);
	if (strcmp(si.os_name, "VelumOS"))
		return (8);
	if (!nprocs_without_heap())
		return (9);
	return (0);
}

int	sys_selftest(void)
{
	int	rc;

	rc = getrandom_checks();
	if (!rc)
		rc = sysinfo_checks();
	if (rc)
		klog_err("random: controle des appels systeme %d en echec", rc);
	return (rc);
}
