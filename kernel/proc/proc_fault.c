#include "proc_int.h"
#include "velum/irqflags.h"
#include "velum/klog.h"
#include "velum/panic.h"

static const char	*g_exc_names[] = {"#DE division", "#DB débogage", "NMI",
	"#BP point d'arrêt", "#OF débordement", "#BR borne",
	"#UD instruction invalide", "#NM FPU absente", "#DF double faute",
	"segment FPU", "#TS TSS invalide", "#NP segment absent", "#SS pile",
	"#GP protection générale", "#PF faute de page", "réservée",
	"#MF FPU x87", "#AC alignement", "#MC machine", "#XM SIMD",
	"#VE virtualisation", "#CP contrôle de flot"};

static const char	*exc_name(uint64_t vec)
{
	if (vec < sizeof(g_exc_names) / sizeof(g_exc_names[0]))
		return (g_exc_names[vec]);
	return ("exception");
}

void	proc_fault(t_regs *regs, uint64_t vec, uint64_t addr)
{
	t_process	*p;

	p = proc_current();
	if (!p)
		panic_regs(regs, "faute en anneau 3 sans processus");
	klog_warn("processus %s (%u) : %s rip %#llx addr %#llx", p->name,
		p->pid, exc_name(vec), (unsigned long long)regs->rip,
		(unsigned long long)addr);
	irq_enable();
	proc_exit_current(-1);
}
