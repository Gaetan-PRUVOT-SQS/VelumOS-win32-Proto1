#include "harness.h"
#include "smp_fixture.h"

#define CPUS 4
#define FILS 64
#define TOURS 300

typedef struct s_cpuctx
{
	t_cpusched	*cs;
	uint64_t	faits;
}	t_cpuctx;

static t_cpusched	g_cs[CPUS];
static t_cpusched	*g_tab[CPUS];
static t_tcase		*g_cas[FILS];
static t_cpuctx		g_ctx[CPUS];
static uint64_t		g_total;

static void	executer(t_tcase *c)
{
	if (__atomic_exchange_n(&c->running, 1, __ATOMIC_ACQUIRE))
		__atomic_fetch_add(&c->overlap, 1, __ATOMIC_RELAXED);
	c->ran++;
	__atomic_store_n(&c->running, 0, __ATOMIC_RELEASE);
}

static bool	un_tour(t_cpuctx *x)
{
	t_kthread	*kt;
	uint64_t	flags;

	flags = spin_lock_irqsave(&x->cs->lock);
	kt = runq_pop_top(&x->cs->rq);
	spin_unlock_irqrestore(&x->cs->lock, flags);
	if (!kt)
		return (false);
	executer((t_tcase *)kt);
	x->faits++;
	__atomic_fetch_add(&g_total, 1, __ATOMIC_RELAXED);
	if (((t_tcase *)kt)->ran >= TOURS)
		return (true);
	flags = spin_lock_irqsave(&x->cs->lock);
	kt->home = x->cs->id;
	runq_push(&x->cs->rq, kt, false);
	spin_unlock_irqrestore(&x->cs->lock, flags);
	return (true);
}

static void	cpu_boucle(void *arg)
{
	t_cpuctx	*x;

	x = arg;
	while (__atomic_load_n(&g_total, __ATOMIC_RELAXED) < FILS * TOURS)
	{
		if (!un_tour(x) && !sched_pull(g_tab, CPUS, x->cs))
			fh_yield();
	}
}

static void	reset(void)
{
	uint32_t	i;

	i = 0;
	while (i < CPUS)
	{
		cs_new(&g_cs[i], i);
		g_tab[i] = &g_cs[i];
		g_ctx[i].cs = &g_cs[i];
		g_ctx[i].faits = 0;
		i++;
	}
	g_total = 0;
	i = 0;
	while (i < FILS)
	{
		g_cas[i] = tcase_new(8, 0);
		runq_push(&g_cs[0].rq, &g_cas[i]->kt, false);
		i++;
	}
}

static void	verifier_et_liberer(void)
{
	uint32_t	i;
	uint64_t	vols;
	uint32_t	doubles;
	uint32_t	manques;

	doubles = 0;
	manques = 0;
	i = 0;
	while (i < FILS)
	{
		doubles += (uint32_t)g_cas[i]->overlap;
		manques += (g_cas[i]->ran != TOURS);
		free(g_cas[i++]);
	}
	vols = 0;
	i = 0;
	while (i < CPUS)
		vols += g_cs[i++].steals;
	h_eq_u64("aucune exécution simultanée du même fil", doubles, 0);
	h_eq_u64("chaque fil exécuté exactement TOURS fois", manques, 0);
	h_eq_u64("total des exécutions", g_total, (uint64_t)FILS * TOURS);
	h_true(vols > 0, "des vols ont eu lieu");
}

static void	files_vides_a_la_fin(void)
{
	uint32_t	i;
	uint32_t	restes;

	restes = 0;
	i = 0;
	while (i < CPUS)
	{
		restes += g_cs[i].rq.count;
		h_eq_u64("verrou rendu", g_cs[i].lock.ticket, g_cs[i].lock.serving);
		i++;
	}
	h_eq_u64("aucun fil perdu ni dupliqué : files vides", restes, 0);
}

static void	vol_sans_perte_ni_doublon(void)
{
	void		*h[CPUS];
	uint32_t	i;

	reset();
	i = 0;
	while (i < CPUS)
	{
		h[i] = fh_spawn(cpu_boucle, &g_ctx[i]);
		i++;
	}
	i = 0;
	while (i < CPUS)
		fh_join(h[i++]);
	files_vides_a_la_fin();
	verifier_et_liberer();
}

int	main(void)
{
	h_begin("e08/course");
	h_run("4 cœurs simulés, 64 fils", vol_sans_perte_ni_doublon);
	h_run("même scénario rejoué", vol_sans_perte_ni_doublon);
	return (h_end());
}
