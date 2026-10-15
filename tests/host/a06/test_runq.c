#include "fake_sched.h"
#include "harness.h"

static t_kthread	g_k[4];

static void	runq_vide(void)
{
	t_runq	rq;

	runq_init(&rq);
	h_eq_i64("sommet d'une file vide", runq_top(&rq), -1);
	h_true(runq_pop_top(&rq) == NULL, "rien à sortir");
	h_eq_u64("compte nul", rq.count, 0);
	h_eq_u64("bitmap nul", rq.bitmap, 0);
}

static void	runq_tourniquet(void)
{
	t_runq	rq;

	runq_init(&rq);
	fu_kt_init(&g_k[0], 8);
	fu_kt_init(&g_k[1], 8);
	fu_kt_init(&g_k[2], 8);
	runq_push(&rq, &g_k[0], false);
	runq_push(&rq, &g_k[1], false);
	runq_push(&rq, &g_k[2], false);
	h_true(runq_pop_top(&rq) == &g_k[0], "queue : premier entré");
	h_true(runq_pop_top(&rq) == &g_k[1], "queue : deuxième");
	runq_push(&rq, &g_k[0], true);
	h_true(runq_pop_top(&rq) == &g_k[0], "tête : passe devant");
	h_true(runq_pop_top(&rq) == &g_k[2], "puis le reste");
	h_eq_u64("vide", rq.count, 0);
}

static void	runq_retrait(void)
{
	t_runq	rq;

	runq_init(&rq);
	fu_kt_init(&g_k[0], 8);
	fu_kt_init(&g_k[1], 8);
	fu_kt_init(&g_k[2], 8);
	runq_push(&rq, &g_k[0], false);
	runq_push(&rq, &g_k[1], false);
	runq_push(&rq, &g_k[2], false);
	runq_remove(&rq, &g_k[1]);
	h_true(g_k[0].t.next == &g_k[2].t && g_k[2].t.prev == &g_k[0].t,
		"milieu : voisins recousus");
	runq_remove(&rq, &g_k[2]);
	h_true(rq.tail[8] == &g_k[0], "queue retirée");
	h_eq_u64("niveau encore marqué", rq.bitmap, 1u << 8);
	runq_remove(&rq, &g_k[0]);
	h_eq_u64("niveau vide démarqué", rq.bitmap, 0);
	h_true(rq.head[8] == NULL && rq.tail[8] == NULL, "liste vide");
	h_eq_u64("compte nul", rq.count, 0);
}

static void	runq_niveaux_limites(void)
{
	t_runq	rq;

	runq_init(&rq);
	fu_kt_init(&g_k[0], 0);
	fu_kt_init(&g_k[1], 31);
	fu_kt_init(&g_k[2], 8);
	fu_kt_init(&g_k[3], 40);
	runq_push(&rq, &g_k[0], false);
	runq_push(&rq, &g_k[1], false);
	runq_push(&rq, &g_k[2], false);
	runq_push(&rq, &g_k[3], false);
	h_eq_i64("priorité 40 ramenée à 31", g_k[3].t.prio, 31);
	h_eq_i64("sommet 31", runq_top(&rq), 31);
	h_true(runq_pop_top(&rq) == &g_k[1], "31 d'abord");
	h_true(runq_pop_top(&rq) == &g_k[3], "puis l'autre 31");
	h_true(runq_pop_top(&rq) == &g_k[2], "puis 8");
	h_true(runq_pop_top(&rq) == &g_k[0], "puis 0");
	g_k[0].t.prio = -3;
	runq_push(&rq, &g_k[0], false);
	h_eq_i64("priorité -3 ramenée à 0", g_k[0].t.prio, 0);
}

int	main(void)
{
	h_begin("a06/runq");
	h_run("file vide", runq_vide);
	h_run("tourniquet tête et queue", runq_tourniquet);
	h_run("retrait tête milieu queue", runq_retrait);
	h_run("niveaux 0 et 31", runq_niveaux_limites);
	return (h_end());
}
