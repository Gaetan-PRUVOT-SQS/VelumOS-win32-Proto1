#include "velum/msr.h"
#include "cpu_int.h"

static void	tlb_flush_all(void)
{
	uint64_t	cr4;

	cr4 = cpu_read_cr4();
	if (cr4 & CR4_PGE)
	{
		cpu_write_cr4(cr4 & ~CR4_PGE);
		cpu_write_cr4(cr4);
		return ;
	}
	cpu_write_cr3(cpu_read_cr3());
}

static void	pat_program(void)
{
	cpu_wbinvd();
	msr_write(MSR_PAT, PAT_VALUE);
	cpu_wbinvd();
	tlb_flush_all();
}

void	cpu_cr_setup(const t_cpufeat *feat)
{
	cpu_write_cr0(cr0_wanted(cpu_read_cr0()));
	if (feat->nx)
		msr_write(MSR_EFER, msr_read(MSR_EFER) | EFER_NXE);
	if (feat->pat)
		pat_program();
	cpu_write_cr4(cr4_wanted(cpu_read_cr4(), feat));
}
