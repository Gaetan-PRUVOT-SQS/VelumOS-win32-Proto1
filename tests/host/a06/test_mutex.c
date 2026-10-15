#include <string.h>
#include "fake_sched.h"
#include "harness.h"

#define MX_THREADS 3
#define MX_LOOPS 100000

static t_futest	g_t;

static void	mx_worker(void *arg)
{
	t_futest	*t;
	int			i;

	t = arg;
	i = 0;
	while (i < MX_LOOPS)
	{
		mutex_lock(&t->m);
		t->counter++;
		mutex_unlock(&t->m);
		i++;
	}
}

static void	mutex_exclusion(void)
{
	void		*h[MX_THREADS];
	int			i;
	uint64_t	parks;

	mutex_init(&g_t.m, "compteur");
	g_t.counter = 0;
	parks = g_ft.parks;
	i = 0;
	while (i < MX_THREADS)
	{
		h[i] = fh_spawn(mx_worker, &g_t);
		i++;
	}
	i = 0;
	while (i < MX_THREADS)
		fh_join(h[i++]);
	h_true((g_ft.parks - parks) * 10 < MX_THREADS * MX_LOOPS * 8,
		"pas de convoi : moins de 8 attentes pour 10 prises");
	h_eq_u64("3 x 100 000 sans perte", g_t.counter, MX_THREADS * MX_LOOPS);
	h_true(g_t.m.owner == NULL, "libre à la fin");
	h_eq_u64("aucun attendeur restant", fu_wq_len(&g_t.m.wq), 0);
}

static void	mx_try_other(void *arg)
{
	t_futest	*t;

	t = arg;
	t->flag = mutex_trylock(&t->m);
}

static void	mutex_trylock_cas(void)
{
	mutex_init(&g_t.m, NULL);
	h_eq_str("nom par défaut", g_t.m.name, "mutex");
	h_true(mutex_trylock(&g_t.m), "libre : pris");
	h_true(g_t.m.owner == &sched_kself()->t, "propriétaire posé");
	g_t.flag = 7;
	fh_join(fh_spawn(mx_try_other, &g_t));
	h_eq_i64("pris par un autre : refusé", g_t.flag, 0);
	mutex_unlock(&g_t.m);
	h_true(g_t.m.owner == NULL, "relâché");
}

int	main(void)
{
	h_begin("a06/mutex");
	h_run("exclusion 3 fils", mutex_exclusion);
	h_run("trylock", mutex_trylock_cas);
	return (h_end());
}
