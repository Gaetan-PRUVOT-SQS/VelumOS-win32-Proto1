#include <string.h>
#include "fake_sched.h"
#include "harness.h"

static void	rang_croissant_accepte(void)
{
	t_spinlock	w;
	t_spinlock	r;

	spin_init(&w, "waitq");
	spin_init(&r, "runq");
	spin_lock(&w);
	spin_lock(&r);
	h_eq_u64("deux verrous suivis", lockdep_depth(), 2);
	spin_unlock(&r);
	spin_unlock(&w);
	h_eq_u64("pile vide", lockdep_depth(), 0);
}

static void	rang_decroissant_panique(void)
{
	t_spinlock	w;
	t_spinlock	r;

	spin_init(&w, "waitq");
	spin_init(&r, "runq");
	spin_lock(&r);
	if (setjmp(*fake_panic_arm()) == 0)
	{
		spin_lock(&w);
		fake_panic_disarm();
		h_true(0, "waitq sous runq : panique attendue");
	}
	else
		h_true(strstr(fake_panic_msg(), "'waitq' (rang 30) pris sous 'runq'")
			!= NULL, "message avec les deux noms");
	fake_reset_self();
}

static void	rang_egal_panique(void)
{
	t_spinlock	a;
	t_spinlock	b;

	spin_init(&a, "waitq");
	spin_init(&b, "waitq");
	spin_lock(&a);
	if (setjmp(*fake_panic_arm()) == 0)
	{
		spin_lock(&b);
		fake_panic_disarm();
		h_true(0, "deux waitq imbriqués : panique attendue");
	}
	else
		h_true(strstr(fake_panic_msg(), "lockdep") != NULL, "message");
	fake_reset_self();
}

static void	classe_inconnue_libre(void)
{
	t_spinlock	u;
	t_spinlock	r;

	spin_init(&u, "pilote_inconnu");
	spin_init(&r, "runq");
	spin_lock(&r);
	spin_lock(&u);
	h_eq_u64("inconnu sous runq accepté", lockdep_depth(), 2);
	spin_unlock(&u);
	spin_unlock(&r);
	spin_lock(&u);
	spin_lock(&r);
	h_eq_u64("runq sous inconnu accepté", lockdep_depth(), 2);
	spin_unlock(&r);
	spin_unlock(&u);
	h_eq_u64("rang d'une classe inconnue", lockdep_rank_of("pilote_inconnu"),
		0);
}

int	main(void)
{
	h_begin("a06/lockdep");
	h_run("rang croissant accepté", rang_croissant_accepte);
	h_run("rang décroissant refusé", rang_decroissant_panique);
	h_run("rang égal refusé", rang_egal_panique);
	h_run("classe inconnue non contrôlée", classe_inconnue_libre);
	return (h_end());
}
