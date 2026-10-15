#include <stdio.h>
#include <string.h>
#include "fake_sched.h"
#include "harness.h"
#include "velum/err.h"

static char	g_names[40][24];

static void	enregistrement_partitions(void)
{
	h_eq_i64("nouvelle classe", spin_rank_register("pilote_a", 20), 0);
	h_eq_i64("doublon même rang", spin_rank_register("pilote_a", 20), 0);
	h_eq_i64("doublon autre rang", spin_rank_register("pilote_a", 21),
		E_EXIST);
	h_eq_i64("classe du noyau redéfinie", spin_rank_register("runq", 5),
		E_EXIST);
	h_eq_i64("nom nul", spin_rank_register(NULL, 5), E_INVAL);
	h_eq_i64("nom vide", spin_rank_register("", 5), E_INVAL);
	h_eq_i64("rang 0", spin_rank_register("x", 0), E_INVAL);
	h_eq_i64("rang 1001", spin_rank_register("x", 1001), E_INVAL);
	h_eq_i64("rang 1000", spin_rank_register("x1000", 1000), 0);
	h_eq_u64("rang relu", lockdep_rank_of("pilote_a"), 20);
	h_eq_u64("nom nul sans rang", lockdep_rank_of(NULL), 0);
}

static void	liberation_hors_ordre(void)
{
	t_spinlock	a;
	t_spinlock	b;
	t_spinlock	c;

	spin_init(&a, "a");
	spin_init(&b, "b");
	spin_init(&c, "c");
	spin_lock(&a);
	spin_lock(&b);
	spin_lock(&c);
	spin_unlock(&b);
	h_eq_u64("b retiré du milieu", lockdep_depth(), 2);
	spin_unlock(&a);
	h_eq_u64("a retiré", lockdep_depth(), 1);
	h_true(sched_lockstack()->held[0].lock == &c, "c reste seul");
	spin_unlock(&c);
	h_eq_u64("pile vide", lockdep_depth(), 0);
}

static void	profondeur_maximale(void)
{
	t_spinlock	l[LOCKDEP_DEPTH + 1];
	int			i;

	i = 0;
	while (i <= LOCKDEP_DEPTH)
		spin_init(&l[i++], "profond");
	i = 0;
	while (i < LOCKDEP_DEPTH)
		spin_lock(&l[i++]);
	h_eq_u64("16 verrous tenus", lockdep_depth(), LOCKDEP_DEPTH);
	if (setjmp(*fake_panic_arm()) == 0)
	{
		spin_lock(&l[LOCKDEP_DEPTH]);
		fake_panic_disarm();
		h_true(0, "17e verrou : panique attendue");
	}
	else
		h_true(strstr(fake_panic_msg(), "plus de 16") != NULL, "message");
	fake_reset_self();
}

static void	table_pleine(void)
{
	int	rc;
	int	ok;
	int	i;

	ok = 0;
	rc = 0;
	i = 0;
	while (rc == 0 && i < 40)
	{
		snprintf(g_names[i], sizeof(g_names[i]), "classe%d", i);
		rc = spin_rank_register(g_names[i], 50);
		ok += (rc == 0);
		i++;
	}
	h_eq_i64("table pleine", rc, E_NOSPC);
	h_eq_i64("places restantes remplies", ok, RANK_SLOTS - 7);
}

int	main(void)
{
	h_begin("a06/lockdep_reg");
	h_run("enregistrement de classes", enregistrement_partitions);
	h_run("libération hors ordre", liberation_hors_ordre);
	h_run("profondeur maximale", profondeur_maximale);
	h_run("table de rangs pleine", table_pleine);
	return (h_end());
}
