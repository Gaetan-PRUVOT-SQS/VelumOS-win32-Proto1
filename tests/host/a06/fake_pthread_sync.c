#include <sched.h>
#include <stdlib.h>
#include "fake_pth.h"

t_fhglob	g_fh = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER,
	PTHREAD_ONCE_INIT, 0};

void	fh_lock(void)
{
	pthread_mutex_lock(&g_fh.mtx);
}

void	fh_unlock(void)
{
	pthread_mutex_unlock(&g_fh.mtx);
}

void	fh_wait_cond(void)
{
	pthread_cond_wait(&g_fh.cond, &g_fh.mtx);
}

void	fh_broadcast(void)
{
	pthread_cond_broadcast(&g_fh.cond);
}

void	fh_yield(void)
{
	sched_yield();
}
