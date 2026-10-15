#include "apic_int.h"
#include "../acpi/acpi_int.h"
#include "velum/err.h"
#include "velum/irq.h"
#include "velum/msr.h"
#include "velum/timer.h"

uint32_t	lapic_lvt_nmi(uint16_t mps_flags)
{
	uint32_t	lvt;

	lvt = LAPIC_LVT_NMI;
	if ((mps_flags & ACPI_POL_MASK) == ACPI_POL_LOW)
		lvt |= LAPIC_LVT_LOW;
	return (lvt);
}

static uint32_t	bsp_uid(const t_acpi_info *ai, const t_acpi_extra *x,
	uint32_t id)
{
	uint32_t	k;

	k = 0;
	while (k < ai->ncpus)
	{
		if (ai->lapic_id[k] == id)
			return (x->lapic_uid[k]);
		k++;
	}
	return (ACPI_UID_ALL - 1);
}

void	lapic_nmi_setup(void)
{
	const t_acpi_extra	*x;
	const t_acpi_nmi	*n;
	uint32_t			uid;
	uint32_t			k;

	x = acpi_extra();
	uid = bsp_uid(acpi_info(), x, apic_id());
	k = 0;
	while (k < x->nnmi)
	{
		n = &x->nmi[k];
		if (n->uid == ACPI_UID_ALL || n->uid == uid)
			lapic_write(LAPIC_LVT_LINT0
				+ (LAPIC_LVT_LINT1 - LAPIC_LVT_LINT0) * n->lint,
				lapic_lvt_nmi(n->flags));
		k++;
	}
}

static bool	icr_idle(void *ctx)
{
	(void)ctx;
	return (!(lapic_read(LAPIC_ICR_LO) & LAPIC_ICR_PENDING));
}

int	lapic_self_ipi(uint8_t vec)
{
	if (!lapic_ready())
		return (E_NODEV);
	if (lapic_is_x2())
	{
		msr_write(X2APIC_SELF_IPI, vec);
		return (E_OK);
	}
	lapic_write(LAPIC_ICR_LO, LAPIC_ICR_SELF | vec);
	return (wait_until(icr_idle, NULL, 1000000));
}
