#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/panic.h"
#include "velum/sched.h"
#include "cpu_int.h"
#include "stack_report.h"

#define VEC_DOUBLE_FAULT 8

static uint64_t	g_unhandled[IDT_VECTORS / 64];

void	trap_unhandled(const t_regs *regs)
{
	uint64_t	vec;
	uint64_t	bit;

	vec = regs->vec & (IDT_VECTORS - 1);
	bit = 1ull << (vec & 63);
	if (g_unhandled[vec >> 6] & bit)
		return ;
	g_unhandled[vec >> 6] |= bit;
	klog_warn("cpu: vecteur %llu sans gestionnaire (rip %#llx)", vec,
		regs->rip);
}

static void	report_regs(const t_regs *r)
{
	kprintf("rsi=%#llx rdi=%#llx rbp=%#llx\n", r->rsi, r->rdi, r->rbp);
	kprintf("r8=%#llx r9=%#llx r10=%#llx r11=%#llx\n", r->r8, r->r9,
		r->r10, r->r11);
	kprintf("r12=%#llx r13=%#llx r14=%#llx r15=%#llx\n", r->r12, r->r13,
		r->r14, r->r15);
	kprintf("rflags=%#llx ss=%#llx\n", r->rflags, r->ss);
}

static void	report_pf(uint64_t err, uint64_t cr2)
{
	kprintf("faute de page : adresse %#llx, P=%llu W=%llu U=%llu R=%llu "
		"I=%llu\n", cr2, err & 1, (err >> 1) & 1, (err >> 2) & 1,
		(err >> 3) & 1, (err >> 4) & 1);
}

static void	stack_collect(t_stackinfo *si, const t_regs *regs, uint64_t cr2)
{
	t_cpu		*cpu;
	t_thread	*th;

	si->rsp = (uint64_t)__builtin_frame_address(0);
	if (regs)
		si->rsp = regs->rsp;
	si->cr2 = cr2;
	if (!regs || regs->vec == VEC_DOUBLE_FAULT)
		si->cr2 = cpu_read_cr2();
	cpu = cpu_self();
	si->has_thread = false;
	if (!cpu)
		return ;
	si->gs_kstack = cpu->kstack_top;
	si->irq_depth = cpu->irq_depth;
	si->preempt = cpu->preempt;
	if (cpu->tss)
		si->tss_rsp0 = ((const t_tss *)cpu->tss)->rsp[0];
	th = cpu->current;
	if (!th)
		return ;
	si->has_thread = true;
	si->tid = th->tid;
	si->name = th->name;
	si->base = th->kstack_base;
	si->top = th->kstack_top;
}

static void	chain_origin_words(const t_trapchain *ch, uint64_t base,
				uint64_t top)
{
	const uint64_t	*w;
	uint64_t		rsp;
	uint64_t		i;
	uint64_t		n;

	if (ch->ntail == 0)
		return ;
	rsp = ch->tail[ch->ntail - 1].rsp;
	if ((rsp & 7) != 0 || rsp < base || rsp >= top)
		return ;
	n = (top - rsp) / sizeof(uint64_t);
	if (n > 6)
		n = 6;
	w = (const uint64_t *)rsp;
	kprintf("chaine : origine, %llu mot(s) a rsp %#llx :", n, rsp);
	i = 0;
	while (i < n)
		kprintf(" %#llx", w[i++]);
	kputc('\n');
}

void	cpu_stack_report(const t_regs *regs, uint64_t cr2)
{
	t_stackinfo	si;
	t_trapchain	chain;
	char		text[STACK_REPORT_SIZE];
	uint64_t	rbp;

	memset(&si, 0, sizeof(si));
	si.name = "?";
	stack_collect(&si, regs, cr2);
	stack_format(text, sizeof(text), &si);
	kputs(text);
	if (!si.has_thread)
		return ;
	rbp = (uint64_t)__builtin_frame_address(0);
	if (regs)
		rbp = regs->rbp;
	stack_trap_chain(&chain, rbp, si.base, si.top);
	if (chain.total < 2)
		return ;
	stack_chain_format(text, sizeof(text), &chain);
	kputs(text);
	chain_origin_words(&chain, si.base, si.top);
}

static void	report_trace(const t_regs *regs)
{
	uint64_t	pcs[BT_MAX];
	int			n;
	int			i;

	kprintf("trace : #0 %#llx\n", regs->rip);
	n = cpu_backtrace(regs->rbp, pcs, BT_MAX);
	i = 0;
	while (i < n)
	{
		kprintf("trace : #%d %#llx\n", i + 1, pcs[i]);
		i++;
	}
}

_Noreturn void	trap_kernel_fault(const t_regs *regs, uint64_t cr2)
{
	char	msg[96];

	kprintf("\nexception %s (%s), vecteur %llu, anneau %llu, pile %s\n",
		trap_name(regs->vec), trap_desc(regs->vec), regs->vec, regs->cs & 3,
		cpu_stack_name((uint64_t)regs));
	report_regs(regs);
	if (regs->vec == VEC_PAGE_FAULT)
		report_pf(regs->err, cr2);
	cpu_stack_report(regs, cr2);
	report_trace(regs);
	ksnprintf(msg, sizeof(msg), "%s %s (anneau %llu)", trap_name(regs->vec),
		trap_desc(regs->vec), regs->cs & 3);
	panic_regs(regs, msg);
}
