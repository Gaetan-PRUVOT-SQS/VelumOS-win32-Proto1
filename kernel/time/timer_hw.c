#include "time_int.h"
#include "../../arch/x86_64/apic/apic_int.h"
#include "velum/cpu.h"
#include "velum/err.h"

static int	oneshot_setup(t_timers *t)
{
	uint64_t	lapic_hz;

	lapic_hz = lapic_timer_calibrate();
	if (!lapic_hz || clockconv_init(&t->lapic_per_ns, lapic_hz, NS_PER_S) < 0)
		return (E_NODEV);
	t->mode = TIMER_MODE_ONESHOT;
	lapic_timer_oneshot_mode();
	return (E_OK);
}

int	timer_hw_init(void)
{
	t_timers	*t;
	int			rc;

	t = timers_state();
	theap_init(&t->heap, t->slots, t->idx, TIMERS_MAX);
	t->lock.name = "minuteries";
	if (cpu_features()->tsc_deadline
		&& clock_state()->source == CLOCK_SRC_TSC)
	{
		t->mode = TIMER_MODE_DEADLINE;
		lapic_timer_deadline_mode();
	}
	else
	{
		rc = oneshot_setup(t);
		if (rc < 0)
			return (rc);
	}
	rc = idt_set_handler(VEC_TIMER, timer_irq, NULL);
	if (rc < 0)
		return (rc);
	t->ready = true;
	return (E_OK);
}

uint32_t	timer_mode(void)
{
	return (timers_state()->mode);
}
