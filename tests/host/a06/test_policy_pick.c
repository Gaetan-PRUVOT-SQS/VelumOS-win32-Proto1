#include "fake_sched.h"
#include "harness.h"

static t_kthread	g_k[4];

static void	vieillissement_limites(void)
{
	t_runq	rq;

	runq_init(&rq);
	fu_kt_init(&g_k[0], 8);
	fu_kt_init(&g_k[1], 24);
	fu_kt_init(&g_k[2], 23);
	runq_push(&rq, &g_k[0], false);
	runq_push(&rq, &g_k[1], false);
	runq_push(&rq, &g_k[2], false);
	h_eq_i64("prêt depuis 999 999 999 ns : rien",
		policy_age(&rq, AGING_NS - 1), -1);
	h_eq_i64("prêt depuis 1 s : monté à 23", policy_age(&rq, AGING_NS), 23);
	h_eq_i64("fil vieilli au niveau 23", g_k[0].t.prio, PRIO_AGED);
	h_eq_i64("remontée posée", g_k[0].boost, PRIO_AGED);
	h_eq_i64("temps réel intact", g_k[1].t.prio, 24);
	h_eq_i64("rien à vieillir de plus", policy_age(&rq, 5 * AGING_NS), -1);
	h_true(rq.head[23] == &g_k[2] && rq.tail[23] == &g_k[0],
		"le vieilli passe en queue du niveau 23");
}

static void	choix_rendre_ou_garder(void)
{
	t_runq	rq;

	runq_init(&rq);
	fu_kt_init(&g_k[0], 8);
	fu_kt_init(&g_k[1], 8);
	fu_kt_init(&g_k[3], 0);
	g_k[3].kflags = KT_IDLE;
	g_k[0].t.state = TS_RUNNING;
	g_k[0].t.quantum_left = 100;
	runq_push(&rq, &g_k[1], false);
	h_true(policy_pick(&rq, &g_k[0], SR_PREEMPT, &g_k[3]) == &g_k[0],
		"quantum restant : garde la main");
	g_k[0].t.state = TS_RUNNING;
	h_true(policy_pick(&rq, &g_k[0], SR_YIELD, &g_k[3]) == &g_k[1],
		"yield : l'autre passe");
	h_true(rq.head[8] == &g_k[0], "le cédant attend en queue");
	g_k[1].t.state = TS_RUNNING;
	g_k[1].t.quantum_left = 0;
	g_k[1].boost = 10;
	g_k[1].t.prio = 10;
	h_true(policy_pick(&rq, &g_k[1], SR_PREEMPT, &g_k[3]) == &g_k[0],
		"quantum épuisé : au suivant");
	h_eq_i64("remontée tombée à la fin du quantum", g_k[1].t.prio, 8);
}

static void	choix_bloque_ou_vide(void)
{
	t_runq	rq;

	runq_init(&rq);
	fu_kt_init(&g_k[0], 8);
	fu_kt_init(&g_k[3], 0);
	g_k[3].kflags = KT_IDLE;
	g_k[0].t.state = TS_BLOCKED;
	h_true(policy_pick(&rq, &g_k[0], SR_BLOCK, &g_k[3]) == &g_k[3],
		"bloqué et file vide : inactif");
	h_eq_u64("bloqué non remis en file", rq.count, 0);
	g_k[3].t.state = TS_RUNNING;
	h_true(policy_pick(&rq, &g_k[3], SR_YIELD, &g_k[3]) == &g_k[3],
		"inactif reste inactif");
	h_eq_u64("inactif jamais en file", rq.count, 0);
	g_k[0].t.state = TS_ZOMBIE;
	h_true(policy_pick(&rq, &g_k[0], SR_EXIT, &g_k[3]) == &g_k[3],
		"zombie non remis en file");
}

static void	tic_decisions(void)
{
	t_runq		rq;
	uint64_t	start;

	runq_init(&rq);
	fu_kt_init(&g_k[0], 8);
	fu_kt_init(&g_k[3], 0);
	g_k[3].kflags = KT_IDLE;
	g_k[0].t.quantum_left = QUANTUM_NS;
	start = 0;
	h_true(!policy_tick(&rq, &g_k[0], 10, &start), "seul, quantum restant");
	h_true(policy_tick(&rq, &g_k[0], QUANTUM_NS + 10, &start),
		"quantum épuisé");
	h_true(!policy_tick(&rq, &g_k[3], QUANTUM_NS + 20, &start),
		"inactif et file vide");
	fu_kt_init(&g_k[1], 4);
	runq_push(&rq, &g_k[1], false);
	h_true(policy_tick(&rq, &g_k[3], QUANTUM_NS + 30, &start),
		"inactif et un fil prêt");
	g_k[0].t.quantum_left = QUANTUM_NS;
	h_true(policy_tick(&rq, &g_k[0], AGING_NS, &start),
		"fil affamé monté au-dessus du courant");
}

int	main(void)
{
	h_begin("a06/policy_pick");
	h_run("vieillissement (limites)", vieillissement_limites);
	h_run("choix : garder ou céder", choix_rendre_ou_garder);
	h_run("choix : bloqué, zombie, vide", choix_bloque_ou_vide);
	h_run("décisions du tic", tic_decisions);
	return (h_end());
}
