#include <string.h>
#include "a04_fake.h"

static void	*conc_thread(void *arg)
{
	t_conc		*c;
	uint32_t	i;

	c = arg;
	i = 0;
	while (i < c->ops)
	{
		a04_conc_step(c);
		i++;
	}
	i = 0;
	while (i < 64)
	{
		kfree(c->own[i]);
		c->own[i++] = NULL;
	}
	return (NULL);
}

static void	conc_init(t_conc *c, t_shared *sh, uint64_t seed, uint32_t ops)
{
	uint32_t	id;

	id = c->id;
	memset(c, 0, sizeof(*c));
	c->id = id;
	c->rng.state = seed;
	c->shared = sh;
	c->ops = ops;
}

void	a04_conc_run(uint64_t seed, uint32_t ops)
{
	pthread_t	th[A04_THREADS];
	t_conc		c[A04_THREADS];
	t_shared	sh;
	uint32_t	i;

	a04_fresh();
	memset(&sh, 0, sizeof(sh));
	i = 0;
	while (i < A04_THREADS)
	{
		c[i].id = i;
		conc_init(&c[i], &sh, seed + i * 7919, ops);
		pthread_create(&th[i], NULL, conc_thread, &c[i]);
		i++;
	}
	i = 0;
	while (i < A04_THREADS)
		pthread_join(th[i++], NULL);
	i = 0;
	while (i < A04_SHARE)
		kfree(sh.slot[i++]);
	h_eq_u64("E11 aucune corruption vue par les fils", c[0].bad + c[1].bad
		+ c[2].bad + c[3].bad, 0);
	a04_drain("E11 fin de la charge concurrente");
}
