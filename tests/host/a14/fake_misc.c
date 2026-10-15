#include <pthread.h>
#include <sched.h>
#include <string.h>
#include "fake_int.h"
#include "fake_sys.h"
#include "velum/err.h"
#include "velum/vmisc.h"

static pthread_mutex_t	g_flog_lock = PTHREAD_MUTEX_INITIALIZER;

int64_t	fake_log(const uint64_t *a)
{
	int	i;

	if (a[2] > V_LOG_MAX)
		return (E_INVAL);
	pthread_mutex_lock(&g_flog_lock);
	i = g_fsys.log_count;
	if (i < FAKE_LOG_LINES)
	{
		memcpy(g_fsys.log_lines[i], (const void *)(uintptr_t)a[1], a[2]);
		g_fsys.log_lines[i][a[2]] = '\0';
		g_fsys.log_levels[i] = (uint32_t)a[0];
		g_fsys.log_count++;
	}
	pthread_mutex_unlock(&g_flog_lock);
	return (0);
}

int64_t	fake_getrandom(const uint64_t *a)
{
	uint8_t		*p;
	uint64_t	i;
	uint64_t	state;

	if (a[1] > V_RANDOM_MAX)
		return (E_INVAL);
	state = __atomic_add_fetch(&g_fsys.random_calls, 1, __ATOMIC_SEQ_CST);
	if (g_fsys.random_fail)
		return (E_NOSYS);
	p = (uint8_t *)(uintptr_t)a[0];
	i = 0;
	while (i < a[1])
	{
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		p[i] = (uint8_t)(state >> 56);
		i++;
	}
	return ((int64_t)a[1]);
}

int64_t	fake_yield(const uint64_t *a)
{
	(void)a;
	__atomic_fetch_add(&g_fsys.yield_calls, 1, __ATOMIC_RELAXED);
	sched_yield();
	return (0);
}

int64_t	fake_vprotect(const uint64_t *a)
{
	(void)a;
	return (g_fsys.vprotect_ret);
}

void	fake_deadline(uint64_t timeout_ns, struct timespec *ts)
{
	uint64_t	total;

	clock_gettime(CLOCK_REALTIME, ts);
	if (timeout_ns > 100000000000ull)
		timeout_ns = 100000000000ull;
	total = (uint64_t)ts->tv_nsec + timeout_ns;
	ts->tv_sec += (time_t)(total / 1000000000ull);
	ts->tv_nsec = (long)(total % 1000000000ull);
}
