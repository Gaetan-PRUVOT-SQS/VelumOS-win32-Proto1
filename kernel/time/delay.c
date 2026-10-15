#include "time_int.h"
#include "velum/io.h"
#include "velum/irqflags.h"

#define IO_DELAY_PORT 0x80

void	time_delay_ns(uint64_t ns)
{
	uint64_t	end;
	uint64_t	steps;

	if (!clock_ready())
	{
		steps = ns / NS_PER_US + 1;
		while (steps > 0)
		{
			outb(IO_DELAY_PORT, 0);
			steps--;
		}
		return ;
	}
	end = time_now_ns();
	if (ns > UINT64_MAX - end)
		end = UINT64_MAX;
	else
		end += ns;
	while (time_now_ns() < end)
		cpu_relax();
}

uint64_t	clock_tsc_at(uint64_t deadline_ns)
{
	t_clock		*c;
	uint64_t	tsc;
	uint64_t	now;
	uint64_t	delta;

	c = clock_state();
	tsc = tsc_read();
	now = clockconv_apply(&c->to_ns, tsc - c->origin);
	if (deadline_ns <= now)
		return (tsc + 1);
	delta = clockconv_apply(&c->tsc_per_ns, deadline_ns - now) + 1;
	if (delta > UINT64_MAX - tsc)
		return (UINT64_MAX);
	return (tsc + delta);
}
