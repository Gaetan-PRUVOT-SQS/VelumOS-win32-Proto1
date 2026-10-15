#include "irq_int.h"
#include "../../arch/x86_64/apic/apic_int.h"
#include "../../arch/x86_64/acpi/acpi_int.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/sched.h"

static t_irq_state	g_irq;

t_irq_state	*irq_state(void)
{
	return (&g_irq);
}

static void	irq_trampoline(t_regs *regs, void *ctx)
{
	t_irq_action	acts[IRQ_SHARE_MAX];
	uint32_t		n;
	uint32_t		k;
	uint64_t		fl;

	(void)regs;
	fl = a05_lock(&g_irq.lock);
	n = irqc_snapshot(&g_irq.core, (int)(uintptr_t)ctx, acts, IRQ_SHARE_MAX);
	a05_unlock(&g_irq.lock, fl);
	k = 0;
	while (k < n)
	{
		acts[k].fn(acts[k].ctx);
		k++;
	}
	apic_eoi();
	sched_irq_exit();
}

static void	irq_spurious(t_regs *regs, void *ctx)
{
	(void)regs;
	(void)ctx;
}

static uint32_t	claim_vectors(void)
{
	int			idx;
	int			rc;
	uint32_t	foreign;

	idx = 0;
	foreign = 0;
	while (idx < IRQ_NVEC)
	{
		rc = idt_set_handler((uint8_t)(VEC_IRQ_BASE + idx), irq_trampoline,
				(void *)(uintptr_t)idx);
		if (rc < 0)
		{
			g_irq.core.slot[idx].used = 1;
			g_irq.core.slot[idx].foreign = 1;
			foreign++;
		}
		idx++;
	}
	return (foreign);
}

int	irq_boot_init(void)
{
	int			rc;
	int			nio;
	uint32_t	foreign;

	irqc_init(&g_irq.core);
	g_irq.lock.name = "irq";
	if (!acpi_extra()->have_madt || (acpi_extra()->madt_flags
			& MADT_PCAT_COMPAT))
		pic_disable();
	rc = lapic_init();
	if (rc < 0)
		return (rc);
	if (idt_set_handler(VEC_SPURIOUS, irq_spurious, NULL) < 0)
		klog_warn("irq: vecteur parasite %#x déjà pris", VEC_SPURIOUS);
	foreign = claim_vectors();
	nio = ioapic_init_all(acpi_info());
	g_irq.ready = true;
	klog_info("irq: LAPIC id %u (x2APIC %u), %d IOAPIC, vecteurs %#x-%#x, "
		"%u déjà pris", apic_id(), lapic_is_x2(), nio, VEC_IRQ_BASE,
		VEC_IRQ_BASE + IRQ_NVEC - 1, foreign);
	return (E_OK);
}
