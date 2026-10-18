#include "smp_int.h"
#include "../apic/apic_int.h"
#include "velum/msr.h"
#include "velum/smp.h"

t_smp	g_smp;

static void	ap_tables(uint32_t idx, uint64_t stack_top, uint8_t *ist)
{
	t_cpu		*cpu;
	uint64_t	tops[IST_COUNT];
	t_dtr		gdtr;
	uint32_t	i;

	cpu = &g_smp.cpu[idx];
	cpu->self = cpu;
	cpu->id = idx;
	cpu->apic_id = g_smp.apic[idx];
	cpu->tss = &g_smp.tss[idx];
	cpu->kstack_top = stack_top;
	i = 0;
	while (i < IST_COUNT)
	{
		tops[i] = (uint64_t)ist + (uint64_t)(i + 1) * IST_SIZE - IST_RESERVE;
		*(t_cpu **)tops[i] = cpu;
		i++;
	}
	tss_fill(&g_smp.tss[idx], tops[0], tops[1], tops[2]);
	g_smp.tss[idx].rsp[0] = stack_top;
	gdt_fill(&g_smp.gdt[idx], (uint64_t)&g_smp.tss[idx]);
	gdtr.limit = sizeof(t_gdt) - 1;
	gdtr.base = (uint64_t)&g_smp.gdt[idx];
	gdt_load(&gdtr);
	tss_load(GDT_TSS);
	msr_write(MSR_GS_BASE, (uint64_t)cpu);
	msr_write(MSR_KERNEL_GS_BASE, 0);
}

_Noreturn void	smp_ap_main(uint64_t idx)
{
	cpu_write_cr3(g_smp.cr3);
	ap_tables((uint32_t)idx, g_smp.stack_top[idx], g_smp.ist[idx]);
	idt_load(&g_smp.idtr);
	cpu_cr_setup(cpu_features());
	cpu_fpu_boot(cpu_raw());
	cpu_spec_apply_local();
	lapic_init_ap();
	__atomic_add_fetch(&g_smp.online, 1, __ATOMIC_ACQ_REL);
	__atomic_store_n(&g_smp.started, (uint32_t)idx, __ATOMIC_RELEASE);
	while (1)
		__asm__ volatile("sti; hlt" : : : "memory");
}
