#include <stdlib.h>
#include <time.h>
#include "fake_pth.h"

static void	*fh_entry(void *p)
{
	t_fhstart	*s;

	s = p;
	s->fn(s->arg);
	return (NULL);
}

void	*fh_spawn(t_fhfn fn, void *arg)
{
	t_fhstart	*s;

	s = calloc(1, sizeof(*s));
	if (!s)
		abort();
	s->fn = fn;
	s->arg = arg;
	if (pthread_create(&s->th, NULL, fh_entry, s) != 0)
		abort();
	return (s);
}

void	fh_join(void *h)
{
	t_fhstart	*s;

	s = h;
	pthread_join(s->th, NULL);
	free(s);
}

void	fh_sleep_us(uint64_t us)
{
	struct timespec	ts;

	ts.tv_sec = (time_t)(us / 1000000);
	ts.tv_nsec = (long)(us % 1000000) * 1000;
	nanosleep(&ts, NULL);
}

uint64_t	fh_now_ns(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec);
}
