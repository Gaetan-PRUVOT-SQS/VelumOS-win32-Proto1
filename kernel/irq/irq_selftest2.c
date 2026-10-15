#include "irq_int.h"
#include "../../arch/x86_64/apic/apic_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/klog.h"
#include "velum/timer.h"

#define IPI_WAIT_NS 100000000ull

static void	ipi_hit(void *ctx)
{
	__atomic_add_fetch((uint32_t *)ctx, 1, __ATOMIC_RELAXED);
}

static bool	ipi_seen(void *ctx)
{
	return (__atomic_load_n((uint32_t *)ctx, __ATOMIC_RELAXED) != 0);
}

int	irq_test_self_ipi(void)
{
	uint32_t	hits;
	uint64_t	fl;
	int			vec;
	int			rc;

	hits = 0;
	vec = irq_vector_alloc(ipi_hit, &hits);
	if (vec < 0)
		return (1);
	fl = irq_save();
	rc = lapic_self_ipi((uint8_t)vec);
	irq_enable();
	if (rc == E_OK)
		rc = wait_until(ipi_seen, &hits, IPI_WAIT_NS);
	irq_restore(fl);
	irq_vector_free(vec);
	hits = __atomic_load_n(&hits, __ATOMIC_RELAXED);
	if (rc != E_OK || hits != 1)
		klog_err("irq: IPI vers soi : rc %d, %u passage(s)", rc, hits);
	return (rc != E_OK || hits != 1);
}
