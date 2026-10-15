#include "../../../kernel/time/time_int.h"
#include "../acpi/acpi_int.h"
#include "velum/err.h"
#include "velum/io.h"
#include "velum/irqflags.h"
#include "velum/klog.h"

#define PM1_SCI_EN 0x0001
#define PM1_SLP_TYP_SHIFT 10
#define PM1_SLP_TYP_MASK 0x1c00
#define PM1_SLP_EN 0x2000
#define PM1_KEEP_MASK 0xc3ff
#define ACPI_ENABLE_TIMEOUT_NS 3000000000ull
#define POWER_SETTLE_NS 2000000000ull

static bool	sci_enabled(void *ctx)
{
	return ((inw(*(const uint16_t *)ctx) & PM1_SCI_EN) != 0);
}

static void	acpi_take_ownership(const t_acpi_info *i, const t_acpi_extra *x)
{
	uint16_t	port;

	port = i->pm1a_cnt;
	if (sci_enabled(&port) || !x->smi_cmd || !x->acpi_enable)
		return ;
	outb(x->smi_cmd, x->acpi_enable);
	if (wait_until(sci_enabled, &port, ACPI_ENABLE_TIMEOUT_NS) < 0)
		klog_warn("power: SCI_EN toujours nul après ACPI_ENABLE");
}

static uint16_t	pm1_prepare(uint16_t port, uint16_t typ)
{
	uint16_t	v;

	v = inw(port) & PM1_KEEP_MASK;
	v |= (uint16_t)((typ << PM1_SLP_TYP_SHIFT) & PM1_SLP_TYP_MASK);
	outw(port, v);
	return (v);
}

static void	pm1_enter_s5(const t_acpi_info *i)
{
	uint16_t	va;
	uint16_t	vb;

	va = pm1_prepare(i->pm1a_cnt, i->slp_typ_a);
	vb = 0;
	if (i->pm1b_cnt)
		vb = pm1_prepare(i->pm1b_cnt, i->slp_typ_b);
	outw(i->pm1a_cnt, va | PM1_SLP_EN);
	if (i->pm1b_cnt)
		outw(i->pm1b_cnt, vb | PM1_SLP_EN);
}

int	power_off(void)
{
	const t_acpi_info	*i;
	uint64_t			fl;

	i = acpi_info();
	if (!i->poweroff_ok)
	{
		klog_err("power: extinction ACPI non supportée sur cette machine");
		return (E_NOTSUP);
	}
	klog_info("power: extinction (ACPI S5, SLP_TYP %u/%u)", i->slp_typ_a,
		i->slp_typ_b);
	fl = irq_save();
	acpi_take_ownership(i, acpi_extra());
	pm1_enter_s5(i);
	time_delay_ns(POWER_SETTLE_NS);
	irq_restore(fl);
	klog_err("power: la machine ne s'est pas éteinte");
	return (E_IO);
}
