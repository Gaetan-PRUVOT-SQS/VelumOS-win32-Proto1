#include <pthread.h>
#include <stdint.h>
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

#define RT_THREADS 4
#define RT_DRAWS 10000

static int	g_bad[RT_THREADS];

static void	*rand_worker(void *arg)
{
	int	*bad;
	int	i;

	bad = arg;
	i = 0;
	while (i < RT_DRAWS)
	{
		*bad += rand() < 0;
		i++;
	}
	return (NULL);
}

static void	rand_threads(void)
{
	pthread_t	th[RT_THREADS];
	uint64_t	start;
	int			i;
	int			bad;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	start = g_fsys.random_calls;
	i = 0;
	while (i < RT_THREADS)
	{
		pthread_create(&th[i], NULL, rand_worker, &g_bad[i]);
		i++;
	}
	bad = 0;
	while (i--)
	{
		pthread_join(th[i], NULL);
		bad += g_bad[i];
	}
	h_eq_i64("4 fils x 10 000 tirages dans [0, RAND_MAX]", bad, 0);
	h_true(g_fsys.random_calls - start >= 600 && g_fsys.random_calls < 700,
		"un appel noyau par tampon de 64 tirages");
}

int	main(void)
{
	h_begin("a14/rand_mt");
	h_run("rand/concurrence : 4 fils", rand_threads);
	return (h_end());
}
