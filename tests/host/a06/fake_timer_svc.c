#include "fake_sched.h"

static int	ft_find_due(uint64_t now)
{
	int	i;

	i = 0;
	while (i < FT_SLOTS)
	{
		if (g_ft.slot[i].state == FT_ARMED && g_ft.slot[i].deadline <= now)
			return (i);
		i++;
	}
	return (-1);
}

static bool	ft_fire_one(void)
{
	int			i;
	t_timerfn	fn;
	void		*ctx;

	fh_lock();
	i = ft_find_due(fh_now_ns());
	if (i < 0)
	{
		fh_unlock();
		return (false);
	}
	g_ft.slot[i].state = FT_FIRING;
	fn = g_ft.slot[i].fn;
	ctx = g_ft.slot[i].ctx;
	fh_unlock();
	fn(ctx);
	fh_lock();
	g_ft.slot[i].state = FT_FREE;
	fh_unlock();
	return (true);
}

static void	ft_service(void *arg)
{
	bool	fired;

	(void)arg;
	while (1)
	{
		fh_sleep_us(200);
		fired = true;
		while (fired)
			fired = ft_fire_one();
	}
}

void	ft_start(void)
{
	bool	go;

	fh_lock();
	go = !g_ft.started;
	g_ft.started = 1;
	fh_unlock();
	if (go)
		g_ft.service = fh_spawn(ft_service, NULL);
}
