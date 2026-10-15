#include <pthread.h>
#include <sched.h>
#include <stdint.h>

uint64_t	th_spawn(void *(*fn)(void *), void *arg)
{
	pthread_t	t;

	pthread_create(&t, NULL, fn, arg);
	return ((uint64_t)t);
}

void	th_join(uint64_t t)
{
	pthread_join((pthread_t)t, NULL);
}

void	th_relax(void)
{
	sched_yield();
}
