#include "fake.h"
#include "velum/acpi.h"
#include "velum/err.h"
#include "velum/irq.h"
#include "velum/timer.h"

const t_acpi_info	*acpi_info(void)
{
	if (g_fake.acpi_absent)
		return (NULL);
	return (&g_fake.acpi);
}

uint32_t	apic_id(void)
{
	return (g_fake.apic);
}

int	wait_until(t_condfn cond, void *ctx, uint64_t timeout_ns)
{
	int	i;

	(void)timeout_ns;
	i = 0;
	while (i < 1000)
	{
		if (cond(ctx))
			return (0);
		i++;
	}
	return (E_TIMEOUT);
}
