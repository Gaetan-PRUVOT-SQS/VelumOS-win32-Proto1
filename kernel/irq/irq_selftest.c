#include "irq_int.h"
#include "../../arch/x86_64/apic/apic_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/klog.h"

#define SHARED_LOW 0x7

static void	count_hit(void *ctx)
{
	(*(int *)ctx)++;
}

static void	other_hit(void *ctx)
{
	(*(int *)ctx)--;
}

static int	test_refusals(uint32_t gsi)
{
	int	fails;
	int	hits;

	fails = (irq_request(0xfffffff0u, count_hit, &hits, 0) != E_INVAL);
	fails += (irq_request(gsi, NULL, &hits, 0) != E_INVAL);
	fails += (irq_request(gsi, count_hit, &hits, 0x80) != E_INVAL);
	fails += (irq_vector_alloc(NULL, NULL) != E_INVAL);
	irq_vector_free(-1);
	irq_vector_free(VEC_SPURIOUS);
	return (fails);
}

static int	test_lines(uint32_t gsi)
{
	int	fails;
	int	h;

	fails = (irq_request(gsi, count_hit, &h, 0) != E_OK);
	fails += (irq_request(gsi, other_hit, &h, 0) != E_BUSY);
	fails += (irq_request(gsi, other_hit, &h, IRQF_SHARED) != E_BUSY);
	irq_free(gsi, count_hit);
	fails += (irq_request(gsi, count_hit, &h, SHARED_LOW) != E_OK);
	fails += (irq_request(gsi, other_hit, &h, SHARED_LOW) != E_OK);
	fails += (irq_request(gsi, count_hit, &h, SHARED_LOW) != E_EXIST);
	irq_free(gsi, count_hit);
	irq_free(gsi, other_hit);
	fails += (irq_request(gsi, other_hit, &h, 0) != E_OK);
	irq_free(gsi, other_hit);
	return (fails);
}

int	irq_selftest(void)
{
	uint32_t	end;
	uint64_t	fl;
	int			fails;

	end = ioapic_gsi_end();
	fl = irq_save();
	fails = test_refusals(end);
	if (end > 0)
		fails += test_lines(end - 1);
	irq_restore(fl);
	fails += irq_test_self_ipi();
	if (fails)
		klog_err("irq: autotest, %d échec(s)", fails);
	return (fails);
}
