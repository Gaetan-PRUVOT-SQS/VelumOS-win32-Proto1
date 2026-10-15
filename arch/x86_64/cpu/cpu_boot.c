#include "velum/boot.h"
#include "velum/klog.h"
#include "cpu_int.h"
#include "cpu_vuln.h"

static t_cpuboot	g_boot;

const t_cpufeat	*cpu_features(void)
{
	return (&g_boot.feat);
}

const t_cpuid_raw	*cpu_raw(void)
{
	return (&g_boot.raw);
}

static void	cpu_log(void)
{
	const t_cpufeat	*f;
	char			flags[160];

	f = &g_boot.feat;
	cpufeat_flags(f, flags, sizeof(flags));
	klog_info("cpu: %s %u/%u/%u \"%s\", pa %u va %u, fpu %llu o, %s",
		f->vendor, f->family, f->model, f->stepping, f->brand, f->phys_bits,
		f->virt_bits, cpu_fpu_area_size(), flags);
}

static void	cpu_fault_option(void)
{
	const char	*word;
	int			fault;

	word = boot_cmdline_get("fault");
	if (!word)
		return ;
	fault = fault_parse(word);
	if (!CPU_FAULT_INJECT || fault == FAULT_NONE)
	{
		klog_warn("cpu: option fault=%s ignorée", word);
		return ;
	}
	cpu_fault_set(fault);
}

int	cpu_boot_init(void)
{
	cpuid_collect(&g_boot.raw, cpu_cpuid);
	cpuid_decode(&g_boot.raw, &g_boot.feat);
	cpu_tables_init(0, g_boot.ist);
	cpu_self()->apic_id = g_boot.raw.r[CPUID_L1][R_EBX] >> 24;
	idt_setup();
	cpu_cr_setup(&g_boot.feat);
	cpu_fpu_boot(&g_boot.raw);
	cpu_fault_option();
	cpu_log();
	cpu_vuln_log();
	cpu_hard_boot();
	return (0);
}
