#include "fake_sched.h"
#include "velum/err.h"
#include "velum/timer.h"

t_ftab	g_ft;

uint64_t	time_now_ns(void)
{
	return (fh_now_ns());
}

int64_t	timer_arm(uint64_t deadline_ns, t_timerfn fn, void *ctx)
{
	int		i;
	int64_t	id;

	ft_start();
	fh_lock();
	i = 0;
	while (i < FT_SLOTS && g_ft.slot[i].state != FT_FREE)
		i++;
	if (g_ft.fail || i == FT_SLOTS)
	{
		fh_unlock();
		return (E_NOMEM);
	}
	id = ++g_ft.next_id;
	g_ft.slot[i].deadline = deadline_ns;
	g_ft.slot[i].fn = fn;
	g_ft.slot[i].ctx = ctx;
	g_ft.slot[i].id = id;
	g_ft.slot[i].state = FT_ARMED;
	fh_unlock();
	return (id);
}

bool	timer_cancel(int64_t id)
{
	int		i;
	bool	done;

	done = false;
	fh_lock();
	i = 0;
	while (i < FT_SLOTS)
	{
		if (g_ft.slot[i].id == id && g_ft.slot[i].state == FT_ARMED)
		{
			g_ft.slot[i].state = FT_FREE;
			done = true;
		}
		i++;
	}
	fh_unlock();
	return (done);
}

void	fake_timer_fail(bool fail)
{
	fh_lock();
	g_ft.fail = fail;
	fh_unlock();
}
