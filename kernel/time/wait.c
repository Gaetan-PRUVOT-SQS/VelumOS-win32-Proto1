#include "time_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"

static int	wait_steps(t_condfn cond, void *ctx, uint64_t timeout_ns)
{
	uint64_t	steps;

	steps = timeout_ns / WAIT_STEP_NS;
	while (!cond(ctx))
	{
		if (steps == 0)
			return (E_TIMEOUT);
		time_delay_ns(WAIT_STEP_NS);
		steps--;
	}
	return (E_OK);
}

int	wait_until(t_condfn cond, void *ctx, uint64_t timeout_ns)
{
	uint64_t	deadline;
	uint64_t	now;

	if (!cond)
		return (E_INVAL);
	if (!clock_ready())
		return (wait_steps(cond, ctx, timeout_ns));
	deadline = time_now_ns();
	if (timeout_ns > UINT64_MAX - deadline)
		deadline = UINT64_MAX;
	else
		deadline += timeout_ns;
	while (1)
	{
		now = time_now_ns();
		if (cond(ctx))
			return (E_OK);
		if (now >= deadline)
			return (E_TIMEOUT);
		cpu_relax();
	}
}
