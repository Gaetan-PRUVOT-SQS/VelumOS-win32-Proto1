#include "velum/libk.h"
#include "smp_int.h"
#include "../apic/apic_int.h"
#include "velum/acpi.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/irq.h"
#include "velum/klog.h"
#include "velum/pmm.h"
#include "velum/smp.h"
#include "velum/timer.h"
#include "velum/vmm.h"

typedef struct s_tramp
{
	uint64_t	phys;
	uint8_t		*page;
	t_tramp_cfg	cfg;
}	t_tramp;

static bool	started_is(void *ctx)
{
	return (__atomic_load_n(&g_smp.started, __ATOMIC_ACQUIRE)
		== (uint32_t)(uintptr_t)ctx);
}

static int	tramp_build(t_tramp *t)
{
	uint64_t	*tables[3];
	uint32_t	i;

	t->phys = pmm_alloc_pages(PMM_DRIVER, TRAMP_PAGES, 1, TRAMP_LOW_LIMIT);
	if (!t->phys || t->phys + TRAMP_PAGES * 4096 > TRAMP_LOW_LIMIT)
	{
		if (t->phys)
			pmm_free_pages(t->phys, TRAMP_PAGES, PMM_DRIVER);
		return (E_NOMEM);
	}
	t->page = phys_to_virt(t->phys);
	memset(t->page, 0, TRAMP_PAGES * 4096);
	memcpy(t->page, smp_tramp_start, (size_t)(smp_tramp_end - smp_tramp_start));
	t->cfg.base = t->phys;
	t->cfg.pml4_phys = t->phys + 4096;
	t->cfg.pdpt_phys = t->phys + 2 * 4096;
	t->cfg.pd_phys = t->phys + 3 * 4096;
	t->cfg.off_pm32 = (uint32_t)(smp_tramp_pm32 - smp_tramp_start);
	t->cfg.off_lm64 = (uint32_t)(smp_tramp_lm64 - smp_tramp_start);
	t->cfg.entry = (uint64_t)(uintptr_t)smp_ap_main;
	i = 0;
	while (i < 3)
	{
		tables[i] = (uint64_t *)(t->page + (i + 1) * 4096);
		i++;
	}
	smp_tramp_data(t->page, &t->cfg);
	smp_tramp_tables(tables[0], tables[1], tables[2],
		phys_to_virt(g_smp.cr3 & ~0xfffull), &t->cfg);
	return (E_OK);
}

static int	ap_resources(uint32_t idx)
{
	void	*stack;
	void	*ist;

	stack = vmm_kstack_alloc(SMP_AP_STACK_PAGES);
	ist = vmm_kstack_alloc(SMP_AP_IST_PAGES);
	if (!stack || !ist)
	{
		if (stack)
			vmm_kstack_free(stack, SMP_AP_STACK_PAGES);
		if (ist)
			vmm_kstack_free(ist, SMP_AP_IST_PAGES);
		return (E_NOMEM);
	}
	g_smp.stack_top[idx] = (uint64_t)(uintptr_t)stack
		+ SMP_AP_STACK_PAGES * 4096 - 8;
	g_smp.ist[idx] = ist;
	return (E_OK);
}

static int	ap_wake(const t_tramp *t, uint32_t idx)
{
	uint8_t	vec;

	vec = (uint8_t)(t->phys >> 12);
	smp_tramp_target(t->page, g_smp.stack_top[idx], idx);
	__atomic_store_n(&g_smp.started, 0, __ATOMIC_RELEASE);
	if (lapic_icr_send(g_smp.apic[idx],
			lapic_icr_value(LAPIC_ICR_INIT, 0)) < 0)
		return (E_IO);
	time_delay_ns(SMP_INIT_DELAY_NS);
	lapic_icr_send(g_smp.apic[idx], lapic_icr_value(LAPIC_ICR_SIPI, vec));
	time_delay_ns(SMP_SIPI_DELAY_NS);
	lapic_icr_send(g_smp.apic[idx], lapic_icr_value(LAPIC_ICR_SIPI, vec));
	return (wait_until(started_is, (void *)(uintptr_t)idx,
			SMP_START_TIMEOUT_NS));
}

static uint32_t	ap_start_all(const t_tramp *t, const uint32_t *ids,
	uint32_t n, bool *clean)
{
	uint32_t	i;
	uint32_t	ok;

	ok = 0;
	i = 0;
	while (i < n)
	{
		g_smp.apic[ok + 1] = ids[i];
		if (ap_resources(ok + 1) < 0)
			klog_warn("smp: pas de mémoire pour le cœur apic %u", ids[i]);
		else if (ap_wake(t, ok + 1) < 0)
		{
			klog_warn("smp: le cœur apic %u ne répond pas", ids[i]);
			*clean = false;
			break ;
		}
		else
			ok++;
		i++;
	}
	return (ok);
}

static bool	ipi_received(void *ctx)
{
	return (smp_ipi_count((uint32_t)(uintptr_t)ctx) >= 1);
}

static void	ipi_check(void)
{
	uint32_t	cpu;
	uint32_t	bad;

	bad = 0;
	cpu = 1;
	while (cpu < smp_cpu_count())
	{
		smp_send_ipi(cpu, VEC_IPI);
		if (wait_until(ipi_received, (void *)(uintptr_t)cpu,
				SMP_IPI_TIMEOUT_NS) < 0)
			bad++;
		cpu++;
	}
	if (bad)
		klog_warn("smp: %u cœurs sans réponse à l'IPI", bad);
	else
		klog_info("smp: IPI reçue par chaque cœur");
}

static uint32_t	plan_aps(uint32_t max, uint32_t *ids)
{
	const t_acpi_info	*ai;
	t_plan_in			in;

	ai = acpi_info();
	in.ids = ai->lapic_id;
	in.count = ai->ncpus;
	in.bsp = apic_id();
	in.want = max;
	in.x2 = lapic_is_x2();
	return (smp_plan(&in, ids, CPU_MAX));
}

int	smp_init(uint32_t max)
{
	uint32_t	ids[CPU_MAX];
	uint32_t	n;
	uint32_t	ok;
	t_tramp		t;
	bool		clean;

	if (max == 0)
		return (E_INVAL);
	if (g_smp.inited)
		return (E_BUSY);
	if (!lapic_ready())
		return (E_NOENT);
	g_smp.inited = true;
	g_smp.apic[0] = apic_id();
	g_smp.cr3 = cpu_read_cr3();
	__atomic_store_n(&g_smp.online, 1, __ATOMIC_RELEASE);
	__asm__ volatile("sidt %0" : "=m"(g_smp.idtr));
	smp_ipi_install();
	n = plan_aps(max, ids);
	klog_info("smp: %u cœurs demandés, %u en MADT, %u à démarrer", max,
		acpi_info()->ncpus, n);
	if (n == 0)
		return (E_OK);
	if (tramp_build(&t) < 0)
	{
		klog_warn("smp: pas de page basse pour le trampoline");
		return (E_OK);
	}
	clean = true;
	ok = ap_start_all(&t, ids, n, &clean);
	if (clean)
		pmm_free_pages(t.phys, TRAMP_PAGES, PMM_DRIVER);
	klog_info("smp: %u cœurs en ligne", smp_cpu_count());
	if (ok)
		ipi_check();
	return (E_OK);
}

void	smp_boot_cmdline(void)
{
	const char	*value;
	uint32_t	max;
	int			rc;

	value = boot_cmdline_get("smp");
	if (!value)
		return ;
	if (smp_parse_count(value, &max) < 0)
	{
		klog_warn("smp: valeur '%s' refusée, un seul cœur", value);
		return ;
	}
	rc = smp_init(max);
	if (rc < 0)
		klog_warn("smp: démarrage impossible (%d), un seul cœur", rc);
}
