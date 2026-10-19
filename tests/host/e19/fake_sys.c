#include <stdarg.h>
#include <stdlib.h>
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/random.h"
#include "velum/sync.h"
#include "velum/timer.h"
#include "fake.h"

static uint64_t	g_now = 1000000000ull;
static int		g_fail_after = -1;
static int		g_live;
static uint64_t	g_rng = 0x9e3779b97f4a7c15ull;

void	fake_fail_alloc(int after)
{
	g_fail_after = after;
}

int	fake_allocs(void)
{
	return (g_live);
}

uint64_t	fake_now(void)
{
	return (g_now);
}

uint64_t	time_now_ns(void)
{
	g_now += 50000;
	return (g_now);
}

int	wait_until(t_condfn cond, void *ctx, uint64_t timeout_ns)
{
	uint64_t	end;

	end = g_now + timeout_ns;
	while (!cond(ctx))
	{
		if (g_now >= end)
			return (E_TIMEOUT);
		g_now += 1000000;
	}
	return (0);
}

void	mutex_init(t_mutex *m, const char *name)
{
	m->name = name;
	m->owner = NULL;
}

void	mutex_lock(t_mutex *m)
{
	if (m->owner)
		abort();
	m->owner = (struct s_thread *)m;
}

bool	mutex_trylock(t_mutex *m)
{
	if (m->owner)
		return (false);
	m->owner = (struct s_thread *)m;
	return (true);
}

void	mutex_unlock(t_mutex *m)
{
	m->owner = NULL;
}

void	*kmalloc(size_t size)
{
	if (g_fail_after == 0)
		return (NULL);
	if (g_fail_after > 0)
		g_fail_after--;
	g_live++;
	return (malloc(size));
}

void	kfree(void *p)
{
	if (!p)
		return ;
	g_live--;
	free(p);
}

uint64_t	krandom_u64(void)
{
	g_rng ^= g_rng << 13;
	g_rng ^= g_rng >> 7;
	g_rng ^= g_rng << 17;
	return (g_rng);
}

uint64_t	krandom_below(uint64_t bound)
{
	return (krandom_u64() % bound);
}

void	klog_info(const char *fmt, ...)
{
	(void)fmt;
}

void	klog_warn(const char *fmt, ...)
{
	(void)fmt;
}

void	klog_err(const char *fmt, ...)
{
	(void)fmt;
}
