#include "time_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/klog.h"

#define ARM_DELAY_NS 100000000ull
#define ARM_TOLERANCE_NS 10000000ull
#define ARM_WAIT_NS 1000000000ull

static void	probe_fire(void *ctx)
{
	t_probe	*p;

	p = ctx;
	p->at = time_now_ns();
	__atomic_store_n(&p->fired, 1, __ATOMIC_RELEASE);
}

static bool	probe_fired(void *ctx)
{
	return (__atomic_load_n(&((t_probe *)ctx)->fired, __ATOMIC_ACQUIRE));
}

static int	probe_check(const t_probe *p, uint64_t start)
{
	uint64_t	elapsed;

	elapsed = p->at - start;
	klog_info("time: minuterie de 100 ms déclenchée après %llu ns", elapsed);
	return (elapsed < ARM_DELAY_NS - ARM_TOLERANCE_NS
		|| elapsed > ARM_DELAY_NS + ARM_TOLERANCE_NS);
}

int	timer_test_arm(void)
{
	t_probe		p;
	uint64_t	start;
	uint64_t	fl;
	int64_t		id;
	int			rc;

	p.fired = 0;
	p.at = 0;
	start = time_now_ns();
	id = timer_arm(start + ARM_DELAY_NS, probe_fire, &p);
	if (id <= 0)
		return (1);
	fl = irq_save();
	irq_enable();
	rc = wait_until(probe_fired, &p, ARM_WAIT_NS);
	irq_restore(fl);
	if (rc != E_OK)
	{
		timer_cancel(id);
		klog_err("time: minuterie de 100 ms jamais déclenchée");
		return (1);
	}
	return (probe_check(&p, start));
}
