#include <string.h>
#include "fake_sched.h"
#include "harness.h"
#include "velum/err.h"

#define PC_ITEMS 100000

static t_futest	g_t;

static void	sem_compte(void)
{
	sem_init(&g_t.sem, 2);
	h_eq_i64("jeton 1", sem_wait(&g_t.sem, 0), 0);
	h_eq_i64("jeton 2", sem_wait(&g_t.sem, 0), 0);
	h_eq_i64("vide : E_TIMEOUT", sem_wait(&g_t.sem, 0), E_TIMEOUT);
	sem_post(&g_t.sem);
	h_eq_i64("post sans attendeur : compte 1", g_t.sem.count, 1);
	sem_init(&g_t.sem, -5);
	h_eq_i64("compte négatif ramené à 0", g_t.sem.count, 0);
	g_t.sem.count = INT64_MAX;
	sem_post(&g_t.sem);
	h_eq_i64("compte plafonné", g_t.sem.count, INT64_MAX);
}

static void	sem_passage_de_main(void)
{
	t_fuwaiter	w;
	void		*h;

	sem_init(&g_t.sem, 0);
	memset(&w, 0, sizeof(w));
	w.sem = &g_t.sem;
	w.timeout = TIMEOUT_NONE;
	w.rc = 1;
	h = fh_spawn(fu_sem_waiter, &w);
	fu_wait_len(&g_t.sem.wq, 1);
	sem_post(&g_t.sem);
	fh_join(h);
	h_eq_i64("attendeur servi", w.rc, 0);
	h_eq_i64("jeton donné, pas compté", g_t.sem.count, 0);
}

static void	pc_producer(void *arg)
{
	t_futest	*t;
	int			i;

	t = arg;
	i = 0;
	while (i < PC_ITEMS)
	{
		sem_post(&t->sem);
		i++;
	}
}

static void	prodcons_sans_perte(void)
{
	void	*h;
	int		got;
	int		i;

	sem_init(&g_t.sem, 0);
	h = fh_spawn(pc_producer, &g_t);
	got = 0;
	i = 0;
	while (i < PC_ITEMS)
	{
		got += (sem_wait(&g_t.sem, 2000000000ull) == 0);
		i++;
	}
	fh_join(h);
	h_eq_i64("100 000 jetons reçus", got, PC_ITEMS);
	h_eq_i64("compte final nul", g_t.sem.count, 0);
	h_eq_u64("aucun attendeur", fu_wq_len(&g_t.sem.wq), 0);
}

int	main(void)
{
	h_begin("a06/sem");
	h_run("compte (limites)", sem_compte);
	h_run("passage de main", sem_passage_de_main);
	h_run("producteur consommateur", prodcons_sans_perte);
	return (h_end());
}
