#include "harness.h"
#include "smp_fixture.h"

#define NB 4

static t_cpusched	g_cs[NB];
static t_cpusched	*g_tab[NB];
static t_tcase		*g_cas[64];
static int			g_ncas;

static void	charger(uint32_t cpu, uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		g_cas[g_ncas] = tcase_new(8, 0);
		g_cas[g_ncas]->kt.home = cpu;
		runq_push(&g_cs[cpu].rq, &g_cas[g_ncas]->kt, false);
		g_ncas++;
		i++;
	}
}

static void	reset(void)
{
	uint32_t	i;

	while (g_ncas > 0)
		free(g_cas[--g_ncas]);
	i = 0;
	while (i < NB)
	{
		cs_new(&g_cs[i], i);
		g_tab[i] = &g_cs[i];
		i++;
	}
}

static void	victime_table_de_decision(void)
{
	reset();
	charger(1, 3);
	charger(2, 1);
	charger(3, 2);
	h_eq_u64("inactif 0 : le plus chargé est 1", sched_victim(g_tab, NB, 0, 1), 1);
	h_eq_u64("cœur 1 chargé 3, écart 2 : cœur 3 pas assez chargé",
		sched_victim(g_tab, NB, 1, 2), NB);
	h_eq_u64("cœur 2 charge 1, écart 2 : victime 1",
		sched_victim(g_tab, NB, 2, 2), 1);
	h_eq_u64("écart 0 traité comme 1", sched_victim(g_tab, NB, 0, 0), 1);
	h_eq_u64("cœur hors table", sched_victim(g_tab, NB, 9, 1), NB);
	g_tab[1] = NULL;
	h_eq_u64("cœur absent ignoré", sched_victim(g_tab, NB, 0, 1), 3);
	reset();
	h_eq_u64("tout vide : personne", sched_victim(g_tab, NB, 0, 1), NB);
}

static void	pull_deplace_un_fil(void)
{
	reset();
	charger(1, 2);
	h_true(sched_pull(g_tab, NB, &g_cs[0]), "pull réussi");
	h_eq_u64("victime : 1 restant", g_cs[1].rq.count, 1);
	h_eq_u64("voleur : 1 en file", g_cs[0].rq.count, 1);
	h_eq_u64("reschedule demandé", (uint64_t)g_cs[0].need_resched, 1);
	h_eq_u64("vols comptés", g_cs[0].steals, 1);
	h_eq_u64("nouveau domicile", ((t_kthread *)g_cs[0].rq.head[8])->home, 0);
	h_eq_u64("verrou victime rendu", g_cs[1].lock.ticket, g_cs[1].lock.serving);
	h_eq_u64("verrou voleur rendu", g_cs[0].lock.ticket, g_cs[0].lock.serving);
}

static void	pull_verrou_pris_abandonne(void)
{
	reset();
	charger(1, 2);
	spin_lock(&g_cs[1].lock);
	h_true(!sched_pull(g_tab, NB, &g_cs[0]), "victime verrouillée : abandon");
	h_eq_u64("rien déplacé", g_cs[1].rq.count, 2);
	h_eq_u64("voleur sans verrou résiduel", g_cs[0].lock.ticket,
		g_cs[0].lock.serving);
	spin_unlock(&g_cs[1].lock);
}

static void	pull_rien_si_epingles(void)
{
	reset();
	g_cas[g_ncas] = tcase_new(8, KT_PINNED);
	runq_push(&g_cs[1].rq, &g_cas[g_ncas++]->kt, false);
	h_true(!sched_pull(g_tab, NB, &g_cs[0]), "fil épinglé : pas volé");
	h_eq_u64("toujours chez la victime", g_cs[1].rq.count, 1);
}

static void	placement_charge(void)
{
	reset();
	charger(0, 3);
	charger(1, 1);
	g_cs[2].busy = 1;
	h_eq_u64("moins chargé : cœur 3 (vide, inactif)",
		sched_least_loaded(g_tab, NB), 3);
	g_cs[3].busy = 1;
	h_eq_u64("égalité cœur 1 (1) cœur 2 (1) cœur 3 (1) : le premier",
		sched_least_loaded(g_tab, NB), 1);
	h_eq_u64("charge = file + occupé", sched_load(&g_cs[0]), 3);
	h_eq_u64("cœur inactif pour réveil : aucun",
		sched_idle_peer(g_tab, NB, 0), NB);
	g_cs[3].busy = 0;
	h_eq_u64("cœur inactif : 3", sched_idle_peer(g_tab, NB, 0), 3);
	h_eq_u64("sauf soi-même", sched_idle_peer(g_tab, NB, 3), NB);
}

int	main(void)
{
	h_begin("e08/equilibre");
	h_run("victime : table de décision", victime_table_de_decision);
	h_run("pull déplace un fil", pull_deplace_un_fil);
	h_run("pull abandonne si verrou pris", pull_verrou_pris_abandonne);
	h_run("pull ignore les épinglés", pull_rien_si_epingles);
	h_run("placement par charge", placement_charge);
	reset();
	return (h_end());
}
