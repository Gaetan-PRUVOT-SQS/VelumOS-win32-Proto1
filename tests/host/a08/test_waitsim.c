#include <stdio.h>
#include <stdlib.h>
#include "harness.h"
#include "fake.h"

static t_sim	g_sim;

static int	sim_hook(void *ctx, int forced)
{
	uint32_t	k;

	(void)ctx;
	if (g_sim.budget <= 0 || (!forced && fk_rand() % 3 != 0))
		return (0);
	k = (uint32_t)(fk_rand() % g_sim.n);
	g_sim.budget--;
	g_sim.ever[k] = 1;
	evt_op(g_sim.objs[k], EV_SET);
	return (1);
}

static void	race_build(void)
{
	uint32_t	i;

	g_sim.n = 1 + (uint32_t)(fk_rand() % FK_SIM_OBJS);
	g_sim.mode = (uint32_t)(fk_rand() % 2);
	g_sim.budget = (int)(fk_rand() % 5);
	i = 0;
	while (i < g_sim.n)
	{
		g_sim.ever[i] = (fk_rand() % 4 == 0);
		evt_create((uint32_t)(fk_rand() % 2), (uint32_t)g_sim.ever[i],
			&g_sim.objs[i]);
		i++;
	}
}

static void	race_judge(int rc)
{
	uint32_t	i;
	uint32_t	set;
	uint32_t	ever;

	set = 0;
	ever = 0;
	i = 0;
	while (i < g_sim.n)
	{
		set += sig_signaled(g_sim.objs[i]);
		ever += (g_sim.ever[i] != 0);
		obj_unref(g_sim.objs[i++]);
	}
	if (rc == E_CANCELED && ((g_sim.mode == WAIT_ALL && set == g_sim.n)
			|| (g_sim.mode == WAIT_ANY && set > 0)))
		g_sim.lost++;
	else if (rc == E_CANCELED)
		g_sim.cancels++;
	else if (g_sim.mode == WAIT_ANY && rc >= 0 && (uint32_t)rc < g_sim.n
		&& g_sim.ever[rc])
		g_sim.ok++;
	else if (g_sim.mode == WAIT_ALL && rc == 0 && ever == g_sim.n)
		g_sim.ok++;
	else
		g_sim.budget = -1000;
}

static void	sim_races(void)
{
	int		i;
	int		bad;
	int		rc;

	bad = 0;
	i = 0;
	while (i < FK_RACES)
	{
		race_build();
		g_fk.hook = sim_hook;
		rc = wait_objects(g_sim.objs, g_sim.n, g_sim.mode, TIMEOUT_INF);
		g_fk.hook = NULL;
		race_judge(rc);
		bad += (g_sim.budget == -1000);
		i++;
	}
	printf("courses : %d reussies, %d sans signal, %d perdues, %d fausses, "
		"%llu sommeils\n", g_sim.ok, g_sim.cancels, g_sim.lost, bad,
		(unsigned long long)g_fk.waits);
	h_eq_i64("aucun reveil perdu", g_sim.lost, 0);
	h_eq_i64("aucun resultat faux", bad, 0);
	h_true(g_sim.ok > 0 && g_sim.cancels > 0, "les deux issues vues");
	h_true(g_fk.waits > 0, "des attentes ont dormi");
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	const char	*env;
	uint64_t	seed;

	env = getenv("A08_SEED");
	seed = 0xa08a08;
	if (env)
		seed = strtoull(env, NULL, 0);
	printf("graine A08_SEED=%#llx\n", (unsigned long long)seed);
	fk_reset();
	fk_seed(seed);
	h_begin("a08/waitsim");
	h_run("sim_races", sim_races);
	return (h_end());
}
