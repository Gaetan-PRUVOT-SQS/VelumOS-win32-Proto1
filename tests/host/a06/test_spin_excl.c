#include "fake_sched.h"
#include "harness.h"
#include "velum/irqflags.h"

#define EXCL_THREADS 4
#define EXCL_PER_THREAD 250000

static t_futest	g_t;

static void	excl_worker(void *arg)
{
	t_futest	*t;
	int			i;

	t = arg;
	i = 0;
	while (i < EXCL_PER_THREAD)
	{
		spin_lock(&t->lock);
		t->counter++;
		spin_unlock(&t->lock);
		i++;
	}
}

static void	spin_exclusion_10m(void)
{
	void	*h[EXCL_THREADS];
	int		i;

	spin_init(&g_t.lock, "compteur");
	g_t.counter = 0;
	i = 0;
	while (i < EXCL_THREADS)
	{
		h[i] = fh_spawn(excl_worker, &g_t);
		i++;
	}
	i = 0;
	while (i < EXCL_THREADS)
		fh_join(h[i++]);
	h_eq_u64("1 M incréments sans perte", g_t.counter,
		(uint64_t)EXCL_THREADS * EXCL_PER_THREAD);
	h_eq_u64("verrou libre à la fin", g_t.lock.ticket, g_t.lock.serving);
}

static void	spin_trylock_libre_pris(void)
{
	t_spinlock	l;

	spin_init(&l, "essai");
	h_true(spin_trylock(&l), "libre : pris");
	h_true(!spin_trylock(&l), "pris : refusé");
	h_eq_u64("préemption coupée une fois", sched_preempt_count(), 1);
	h_eq_u64("un verrou suivi", lockdep_depth(), 1);
	spin_unlock(&l);
	h_eq_u64("préemption rendue", sched_preempt_count(), 0);
	h_true(spin_trylock(&l), "relâché : repris");
	spin_unlock(&l);
	h_eq_u64("pile de verrous vide", lockdep_depth(), 0);
}

static void	spin_irqsave_rend_if(void)
{
	t_spinlock	l;
	uint64_t	f;

	spin_init(&l, NULL);
	h_eq_str("nom par défaut", l.name, "?");
	irq_enable();
	f = spin_lock_irqsave(&l);
	h_eq_u64("IF coupé sous le verrou", fake_self()->iflag, 0);
	h_eq_u64("IF d'entrée rendu", f, RFLAGS_IF);
	spin_unlock_irqrestore(&l, f);
	h_eq_u64("IF rétabli", fake_self()->iflag, RFLAGS_IF);
	irq_disable();
	f = spin_lock_irqsave(&l);
	spin_unlock_irqrestore(&l, f);
	h_eq_u64("IF coupé reste coupé", fake_self()->iflag, 0);
	irq_enable();
}

int	main(void)
{
	h_begin("a06/spin_excl");
	h_run("exclusion 1 M", spin_exclusion_10m);
	h_run("trylock libre et pris", spin_trylock_libre_pris);
	h_run("irqsave rend IF", spin_irqsave_rend_if);
	return (h_end());
}
