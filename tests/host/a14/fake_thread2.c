#include "fake_int.h"
#include "fake_sys.h"
#include "velum/err.h"

static t_fthr			g_fthr[FAKE_THREADS];
static pthread_mutex_t	g_fthr_lock = PTHREAD_MUTEX_INITIALIZER;

t_fthr	*fake_thr(int i)
{
	return (&g_fthr[i]);
}

pthread_mutex_t	*fake_thr_lock(void)
{
	return (&g_fthr_lock);
}

int64_t	fake_thread_exit(const uint64_t *a)
{
	(void)a;
	__atomic_fetch_sub(&g_fsys.threads_alive, 1, __ATOMIC_SEQ_CST);
	pthread_exit(NULL);
}

int64_t	fake_thread_close(uint64_t h)
{
	int	i;

	i = (int)(h - FAKE_TH_BASE);
	if (h < FAKE_TH_BASE || i >= FAKE_THREADS)
		return (E_BADF);
	pthread_mutex_lock(&g_fthr_lock);
	if (!g_fthr[i].used)
		i = -1;
	else
		g_fthr[i].used = 0;
	pthread_mutex_unlock(&g_fthr_lock);
	if (i < 0)
		return (E_BADF);
	return (0);
}
