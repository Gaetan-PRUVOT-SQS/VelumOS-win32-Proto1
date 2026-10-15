#include "velum/msr.h"
#include "velum/panic.h"
#include "cpu_int.h"

_Static_assert(sizeof(t_regs) == 176, "t_regs");
_Static_assert(offsetof(t_cpu, self) == CPU_OFF_SELF, "CPU_OFF_SELF");
_Static_assert(offsetof(t_cpu, kstack_top) == CPU_OFF_KSTACK, "CPU_OFF_KSTACK");
_Static_assert(offsetof(t_cpu, user_rsp) == CPU_OFF_USERRSP, "CPU_OFF_USERRSP");
_Static_assert(offsetof(t_cpu, current) == CPU_OFF_CURRENT, "CPU_OFF_CURRENT");

static t_cputab	g_tab;

static void	ist_prepare(t_cpu *cpu, uint8_t *ist, uint64_t tops[IST_COUNT])
{
	int	i;

	i = 0;
	while (i < IST_COUNT)
	{
		tops[i] = (uint64_t)ist + (uint64_t)(i + 1) * IST_SIZE - IST_RESERVE;
		*(t_cpu **)tops[i] = cpu;
		i++;
	}
}

void	cpu_tables_init(uint32_t id, uint8_t *ist)
{
	t_cpu		*cpu;
	t_tss		*tss;
	t_gdt		*gdt;
	uint64_t	tops[IST_COUNT];
	t_dtr		gdtr;

	kassert_check(id < CPU_MAX, "cpu: identifiant hors limites");
	cpu = &g_tab.cpus[id];
	tss = &g_tab.tss[id];
	gdt = &g_tab.gdt[id];
	cpu->self = cpu;
	cpu->id = id;
	cpu->tss = tss;
	ist_prepare(cpu, ist, tops);
	g_tab.ist_base[id] = (uint64_t)ist;
	tss_fill(tss, tops[0], tops[1], tops[2]);
	gdt_fill(gdt, (uint64_t)tss);
	gdtr.limit = sizeof(t_gdt) - 1;
	gdtr.base = (uint64_t)gdt;
	gdt_load(&gdtr);
	tss_load(GDT_TSS);
	msr_write(MSR_GS_BASE, (uint64_t)cpu);
	msr_write(MSR_KERNEL_GS_BASE, 0);
}

void	gdt_set_rsp0(uint64_t rsp0)
{
	t_cpu	*cpu;
	t_tss	*tss;

	cpu = cpu_self();
	tss = cpu->tss;
	tss->rsp[0] = rsp0;
	cpu->kstack_top = rsp0;
}

const char	*cpu_stack_name(uint64_t addr)
{
	static const char *const	names[IST_COUNT] = {"ist1", "ist2", "ist3"};
	uint64_t					base;
	t_cpu						*cpu;

	cpu = cpu_self();
	if (!cpu || cpu->id >= CPU_MAX)
		return ("inconnue");
	base = g_tab.ist_base[cpu->id];
	if (base && addr >= base && addr < base + IST_COUNT * IST_SIZE)
		return (names[(addr - base) / IST_SIZE]);
	return ("noyau");
}

uint32_t	cpu_count(void)
{
	return (1);
}
