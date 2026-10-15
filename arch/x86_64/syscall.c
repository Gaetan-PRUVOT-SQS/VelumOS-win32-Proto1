#include "syscall_int.h"
#include "velum/err.h"
#include "velum/ksyscall.h"
#include "velum/msr.h"
#include "velum/proc.h"

_Static_assert(CPU_OFF_KSTACK == 8, "syscall_entry.S lit gs:[8]");
_Static_assert(CPU_OFF_USERRSP == 16, "syscall_entry.S lit gs:[16]");
_Static_assert(GDT_UDATA == 0x1b && GDT_UCODE == 0x23, "selecteurs user");
_Static_assert(sizeof(t_regs) == 176, "trame de syscall_entry.S");

int	syscall_arch_init(void)
{
	msr_write(MSR_EFER, msr_read(MSR_EFER) | EFER_SCE);
	msr_write(MSR_STAR, SYSCALL_STAR);
	msr_write(MSR_LSTAR, (uint64_t)(uintptr_t) & syscall_entry);
	msr_write(MSR_SFMASK, SYSCALL_SFMASK);
	return (0);
}

int	syscall_arch_selftest(void)
{
	if (!(msr_read(MSR_EFER) & EFER_SCE))
		return (1);
	if (msr_read(MSR_STAR) != SYSCALL_STAR)
		return (2);
	if (msr_read(MSR_LSTAR) != (uint64_t)(uintptr_t) & syscall_entry)
		return (3);
	if (msr_read(MSR_SFMASK) != SYSCALL_SFMASK)
		return (4);
	return (0);
}

static uint32_t	syscall_number(uint64_t rax)
{
	if (rax > UINT32_MAX)
		return (UINT32_MAX);
	return ((uint32_t)rax);
}

int	syscall_entry_c(t_regs *regs)
{
	t_sysargs	args;
	int			path;

	args.a[0] = regs->rdi;
	args.a[1] = regs->rsi;
	args.a[2] = regs->rdx;
	args.a[3] = regs->r10;
	args.a[4] = regs->r8;
	args.a[5] = regs->r9;
	args.regs = regs;
	regs->rax = (uint64_t)syscall_dispatch(&args, syscall_number(regs->rax));
	proc_return_check();
	path = syscall_prepare_return(regs);
	if (path == KILL_PATH)
		proc_exit_current(-1);
	return (path);
}
