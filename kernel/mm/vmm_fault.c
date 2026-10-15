#include "velum/klog.h"
#include "velum/panic.h"
#include "vmm_int.h"
#include "vmm_weak.h"

static const char	*bit(uint64_t err, uint64_t mask, const char *on,
	const char *off)
{
	if (err & mask)
		return (on);
	return (off);
}

static bool	is_stack_guard(uint64_t addr)
{
	t_ptlook	look;

	if (addr < KSTACK_BASE || addr >= KSTACK_BASE + WIN_SIZE)
		return (false);
	return (!vmm_lookup(&g_vmm.kas, addr, &look));
}

static void	kernel_fault(t_regs *regs, uint64_t addr)
{
	char	msg[160];

	if (is_stack_guard(addr))
	{
		ksnprintf(msg, sizeof(msg), "vmm: page de garde de pile noyau "
			"touchee a %#llx (fault=guard)", addr);
		panic_regs(regs, msg);
	}
	ksnprintf(msg, sizeof(msg), "vmm: faute de page noyau a %#llx "
		"[%s%s%s%s%s]", addr, bit(regs->err, 1, "P", "-"),
		bit(regs->err, 2, "W", "R"), bit(regs->err, 4, "U", "S"),
		bit(regs->err, 8, "V", "-"), bit(regs->err, 16, "I", "-"));
	panic_regs(regs, msg);
}

void	vmm_page_fault(t_regs *regs, void *ctx)
{
	uint64_t	addr;

	(void)ctx;
	addr = mmu_read_cr2();
	if ((regs->cs & 3) == 3)
	{
		if (proc_fault)
		{
			proc_fault(regs, VEC_PAGE_FAULT, addr);
			return ;
		}
		panic_regs(regs, "vmm: faute de page utilisateur sans processus");
	}
	kernel_fault(regs, addr);
}
