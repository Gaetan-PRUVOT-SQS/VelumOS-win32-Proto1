#include "velum/klog.h"
#include "stack_report.h"

t_stackpos	stack_position(uint64_t addr, uint64_t base, uint64_t top)
{
	if (base == 0 || top <= base)
		return (SPOS_UNKNOWN);
	if (addr >= base && addr <= top)
		return (SPOS_INSIDE);
	if (addr >= base - STACK_GUARD_SIZE && addr < base)
		return (SPOS_GUARD);
	if (addr < base - STACK_GUARD_SIZE)
		return (SPOS_BELOW);
	return (SPOS_OUTSIDE);
}

const char	*stack_pos_name(t_stackpos pos)
{
	if (pos == SPOS_INSIDE)
		return ("dans la pile");
	if (pos == SPOS_GUARD)
		return ("dans la garde");
	if (pos == SPOS_BELOW)
		return ("sous la garde");
	if (pos == SPOS_OUTSIDE)
		return ("hors pile");
	return ("inconnue");
}

uint64_t	stack_free_bytes(uint64_t rsp, uint64_t base, uint64_t top)
{
	if (stack_position(rsp, base, top) != SPOS_INSIDE)
		return (0);
	return (rsp - base);
}

bool	stack_coherent(const t_stackinfo *si)
{
	return (si->has_thread && si->tss_rsp0 == si->top
		&& si->gs_kstack == si->top);
}

static int	format_thread(char *out, size_t size, const t_stackinfo *si)
{
	if (!si->has_thread)
		return (ksnprintf(out, size, "pile : aucun fil courant\n"));
	return (ksnprintf(out, size, "pile : fil %u (%s), pile noyau %#llx-%#llx "
			"(%llu o), garde %#llx-%#llx, irq_depth %u, preempt %u\n",
			si->tid, si->name, si->base, si->top, si->top - si->base,
			si->base - STACK_GUARD_SIZE, si->base, si->irq_depth,
			si->preempt));
}

static int	format_position(char *out, size_t size, const t_stackinfo *si)
{
	t_stackpos	rsp_pos;
	t_stackpos	cr2_pos;

	rsp_pos = stack_position(si->rsp, si->base, si->top);
	cr2_pos = stack_position(si->cr2, si->base, si->top);
	return (ksnprintf(out, size, "pile : rsp %#llx %s (reste %llu o), cr2 "
			"%#llx %s\n", si->rsp, stack_pos_name(rsp_pos),
			stack_free_bytes(si->rsp, si->base, si->top), si->cr2,
			stack_pos_name(cr2_pos)));
}

static int	format_rsp0(char *out, size_t size, const t_stackinfo *si)
{
	const char	*verdict;

	verdict = "incoherents";
	if (stack_coherent(si))
		verdict = "coherents";
	return (ksnprintf(out, size, "pile : tss.rsp0 %#llx, gs:[8] %#llx, "
			"kstack_top du fil %#llx : %s\n", si->tss_rsp0, si->gs_kstack,
			si->top, verdict));
}

int	stack_format(char *out, size_t size, const t_stackinfo *si)
{
	size_t	len;
	int		n;

	if (!out || !size || !si)
		return (0);
	out[0] = '\0';
	len = 0;
	n = format_thread(out, size, si);
	if (n > 0)
		len = (size_t)n;
	if (len >= size)
		return ((int)(size - 1));
	n = format_position(out + len, size - len, si);
	if (n > 0)
		len += (size_t)n;
	if (len >= size)
		return ((int)(size - 1));
	n = format_rsp0(out + len, size - len, si);
	if (n > 0)
		len += (size_t)n;
	if (len >= size)
		len = size - 1;
	return ((int)len);
}

static bool	frame_plausible(const t_regs *r)
{
	if (r->vec >= 256)
		return (false);
	if (r->cs != STACK_SEL_KERNEL && r->cs != STACK_SEL_USER)
		return (false);
	return ((r->rflags & 2) != 0);
}

static void	chain_push(t_trapchain *ch, const t_regs *r)
{
	t_trapframe	f;
	int			i;

	f.vec = r->vec;
	f.err = r->err;
	f.rip = r->rip;
	f.cs = r->cs;
	f.rsp = r->rsp;
	if (ch->nhead < STACK_CHAIN_MAX)
		ch->head[ch->nhead++] = f;
	if (ch->ntail < STACK_CHAIN_MAX)
		ch->tail[ch->ntail++] = f;
	else
	{
		i = 1;
		while (i < STACK_CHAIN_MAX)
		{
			ch->tail[i - 1] = ch->tail[i];
			i++;
		}
		ch->tail[STACK_CHAIN_MAX - 1] = f;
	}
	ch->total++;
}

void	stack_trap_chain(t_trapchain *out, uint64_t rbp, uint64_t lo,
			uint64_t hi)
{
	const t_regs	*regs;
	uint64_t		next;
	int				steps;

	out->nhead = 0;
	out->ntail = 0;
	out->total = 0;
	steps = 0;
	while (steps++ < STACK_CHAIN_STEPS && (rbp & 7) == 0 && rbp >= lo
		&& rbp <= hi && hi - rbp >= 16 + sizeof(t_regs))
	{
		regs = (const t_regs *)(rbp + 16);
		if (frame_plausible(regs))
			chain_push(out, regs);
		next = *(const uint64_t *)rbp;
		if (next <= rbp)
			break ;
		rbp = next;
	}
}

static int	chain_line(char *out, size_t size, const char *tag,
				const t_trapframe *f)
{
	return (ksnprintf(out, size, "chaine : %s vec %llu err %#llx rip %#llx "
			"cs %#llx rsp %#llx\n", tag, f->vec, f->err, f->rip, f->cs,
			f->rsp));
}

static size_t	chain_append(size_t size, size_t len, int n)
{
	if (n > 0)
		len += (size_t)n;
	if (len >= size)
		len = size - 1;
	return (len);
}

int	stack_chain_format(char *out, size_t size, const t_trapchain *ch)
{
	size_t	len;
	int		i;

	if (!out || !size || !ch)
		return (0);
	out[0] = '\0';
	len = chain_append(size, 0, ksnprintf(out, size, "chaine : %d "
				"trame(s) de trap dans la pile\n", ch->total));
	i = 0;
	while (i < ch->nhead && len + 1 < size)
		len = chain_append(size, len, chain_line(out + len, size - len,
					"interne", &ch->head[i++]));
	i = 0;
	while (ch->total > STACK_CHAIN_MAX && i < ch->ntail && len + 1 < size)
		len = chain_append(size, len, chain_line(out + len, size - len,
					"externe", &ch->tail[i++]));
	return ((int)len);
}
