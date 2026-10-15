#include "velum/klog.h"
#include "velum/msr.h"
#include "cpu_int.h"

int	cpu_st_expect(int cond, const char *what)
{
	if (cond)
		return (0);
	klog_err("cpu: autotest %s KO", what);
	return (1);
}

static int	st_segments(void)
{
	const t_tss	*tss;
	int			fails;

	tss = cpu_self()->tss;
	fails = cpu_st_expect(cpu_read_cs() == GDT_KCODE, "cs = 0x08");
	fails += cpu_st_expect(cpu_read_ss() == GDT_KDATA, "ss = 0x10");
	fails += cpu_st_expect(cpu_read_tr() == GDT_TSS, "tr = 0x28");
	fails += cpu_st_expect(tss->iomap_base == sizeof(t_tss), "bitmap e/s");
	fails += cpu_st_expect(tss->ist[IST_DOUBLE_FAULT - 1] != 0, "ist1");
	return (fails);
}

static int	st_percpu(void)
{
	t_cpu	*cpu;
	int		fails;

	cpu = cpu_self();
	fails = cpu_st_expect(cpu != NULL, "cpu_self");
	if (fails)
		return (fails);
	fails += cpu_st_expect(cpu->self == cpu, "t_cpu.self");
	fails += cpu_st_expect(msr_read(MSR_GS_BASE) == (uint64_t)cpu, "gs");
	fails += cpu_st_expect(cpu->id == 0 && cpu_count() == 1, "cpu 0");
	return (fails);
}

static int	st_control(void)
{
	const t_cpufeat	*f;
	uint64_t		cr4;
	int				fails;

	f = cpu_features();
	cr4 = cpu_read_cr4();
	fails = cpu_st_expect((cpu_read_cr0() & CR0_WP) != 0, "cr0.wp");
	fails += cpu_st_expect(!f->nx || (msr_read(MSR_EFER) & EFER_NXE), "nx");
	fails += cpu_st_expect(!f->smep || (cr4 & CR4_SMEP), "smep");
	fails += cpu_st_expect(!f->smap || (cr4 & CR4_SMAP), "smap");
	fails += cpu_st_expect(!f->umip || (cr4 & CR4_UMIP), "umip");
	fails += cpu_st_expect((cr4 & CR4_OSFXSR) != 0, "osfxsr");
	return (fails);
}

int	cpu_selftest(void)
{
	int	fails;

	fails = st_segments() + st_percpu() + st_control() + cpu_selftest_more();
	if (fails)
		return (fails);
	return (cpu_fault_run());
}
