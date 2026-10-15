#include <string.h>
#include "fake_int.h"
#include "fake_sys.h"
#include "velum/err.h"
#include "velum/vthread.h"

_Noreturn void	__velum_thread_main(t_vthread *thread);
void			__velum_thread_entry(void *arg);

void	__velum_thread_entry(void *arg)
{
	__velum_thread_main(arg);
}

static void	*fake_thread_main(void *slot)
{
	t_fthr	*t;

	t = slot;
	t->entry(t->arg);
	return (NULL);
}

static int	fake_thread_start(int i, const uint64_t *a)
{
	pthread_t	tid;
	t_fthr		*t;

	t = fake_thr(i);
	t->used = 1;
	t->entry = (void (*)(void *))(uintptr_t)a[0];
	t->arg = (void *)(uintptr_t)a[1];
	__atomic_fetch_add(&g_fsys.threads_alive, 1, __ATOMIC_SEQ_CST);
	if (pthread_create(&tid, NULL, fake_thread_main, t))
	{
		t->used = 0;
		__atomic_fetch_sub(&g_fsys.threads_alive, 1, __ATOMIC_SEQ_CST);
		return (-1);
	}
	pthread_detach(tid);
	return (0);
}

int64_t	fake_thread_create(const uint64_t *a)
{
	int	i;

	memcpy(g_fsys.thr_args, a, sizeof(g_fsys.thr_args));
	if (g_fsys.thread_fail)
		return (g_fsys.thread_fail);
	pthread_mutex_lock(fake_thr_lock());
	i = 0;
	while (i < FAKE_THREADS && fake_thr(i)->used)
		i++;
	if (i == FAKE_THREADS || !a[0] || fake_thread_start(i, a) < 0)
		i = -1;
	pthread_mutex_unlock(fake_thr_lock());
	if (i < 0)
		return (E_AGAIN);
	return (FAKE_TH_BASE + i);
}
