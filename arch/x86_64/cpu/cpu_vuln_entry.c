#include "velum/klog.h"
#include "cpu_vuln.h"

_Static_assert(offsetof(t_regs, rax) == 112, "isr.S REGS_RAX");
_Static_assert(offsetof(t_regs, vec) == 120, "isr.S REGS_VEC");
_Static_assert(offsetof(t_regs, err) == 128, "isr.S REGS_ERR");
_Static_assert(offsetof(t_regs, rip) == 136, "isr.S REGS_RIP");
_Static_assert(offsetof(t_regs, cs) == 144, "isr.S REGS_CS");

typedef enum e_smapstate
{
	SMAP_UNTESTED,
	SMAP_OPEN,
	SMAP_BLOCKED,
	SMAP_ABSENT
}	t_smapstate;

uint8_t			g_cpu_entry_audit;
static uint32_t	g_audit_logged;
static uint32_t	g_audit_errors;

void	cpu_entry_audit_enable(void)
{
	g_cpu_entry_audit = 1;
}

bool	cpu_entry_audit_enabled(void)
{
	return (g_cpu_entry_audit != 0);
}

static uint64_t	rflags_now(void)
{
	uint64_t	value;

	__asm__ volatile ("pushfq\n\tpopq %0" : "=r"(value) : : "memory");
	return (value);
}

static t_smapstate	smap_probe(const t_regs *regs)
{
	uint64_t	res;

	if (!cpu_features()->smap || !(cpu_read_cr4() & CR4_SMAP))
		return (SMAP_UNTESTED);
	if (regs->rip < PROBE_USER_MIN || regs->rip >= PROBE_USER_END)
		return (SMAP_UNTESTED);
	res = cpu_probe_user_read(regs->rip);
	if (!(res & PROBE_FAULT))
		return (SMAP_OPEN);
	if ((res & 1) && !(res & 4))
		return (SMAP_BLOCKED);
	return (SMAP_ABSENT);
}

static const char	*smap_text(t_smapstate st)
{
	if (st == SMAP_OPEN)
		return ("ouvert");
	if (st == SMAP_BLOCKED)
		return ("effectif");
	if (st == SMAP_ABSENT)
		return ("page absente");
	return ("non testé");
}

void	cpu_entry_audit(const t_regs *regs)
{
	t_smapstate	st;
	bool		kernel_ac;

	if (!(regs->rflags & RFLAGS_AC))
		return ;
	kernel_ac = (rflags_now() & RFLAGS_AC) != 0;
	st = smap_probe(regs);
	if (kernel_ac || st == SMAP_OPEN)
	{
		if (g_audit_errors++ < AUDIT_LOG_MAX)
			klog_err("cpu: audit entrée KO vec %llu, AC noyau %d, SMAP %s",
				regs->vec, kernel_ac, smap_text(st));
		return ;
	}
	if (g_audit_logged++ < AUDIT_LOG_MAX)
		klog_info("cpu: audit entrée vec %llu, AC utilisateur 1, AC noyau 0, "
			"SMAP %s", regs->vec, smap_text(st));
}
