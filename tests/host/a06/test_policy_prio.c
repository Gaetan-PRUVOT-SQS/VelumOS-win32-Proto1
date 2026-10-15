#include "fake_sched.h"
#include "harness.h"

static t_kthread	g_k[2];

static void	prio_effective_max(void)
{
	fu_kt_init(&g_k[0], 8);
	h_eq_i64("base seule", policy_prio(&g_k[0]), 8);
	g_k[0].boost = 10;
	h_eq_i64("remontée", policy_prio(&g_k[0]), 10);
	g_k[0].inherit = 13;
	h_eq_i64("héritage", policy_prio(&g_k[0]), 13);
	g_k[0].t.base_prio = 30;
	h_eq_i64("base plus haute", policy_prio(&g_k[0]), 30);
	g_k[0].inherit = 40;
	h_eq_i64("plafond 31", policy_prio(&g_k[0]), 31);
}

static void	remontee_limites(void)
{
	static const int32_t	base[6] = {8, 21, 22, 23, 24, 0};
	static const int32_t	want[6] = {10, 23, 23, 23, 0, 2};
	int						i;
	int						ok;

	ok = 0;
	i = 0;
	while (i < 6)
	{
		fu_kt_init(&g_k[0], base[i]);
		policy_wake_boost(&g_k[0]);
		ok += (g_k[0].boost == want[i]);
		i++;
	}
	h_eq_i64("remontées 8, 21, 22, 23, 24 (temps réel), 0", ok, 6);
	fu_kt_init(&g_k[0], 8);
	g_k[0].boost = 23;
	policy_wake_boost(&g_k[0]);
	h_eq_i64("une remontée ne baisse jamais", g_k[0].boost, 23);
	fu_kt_init(&g_k[0], 0);
	g_k[0].kflags = KT_IDLE;
	policy_wake_boost(&g_k[0]);
	h_eq_i64("fil inactif jamais remonté", g_k[0].boost, 0);
}

static void	preemption_table(void)
{
	fu_kt_init(&g_k[0], 8);
	fu_kt_init(&g_k[1], 9);
	h_true(policy_should_preempt(&g_k[0], &g_k[1]), "réveillé plus haut");
	g_k[1].t.prio = 8;
	h_true(!policy_should_preempt(&g_k[0], &g_k[1]), "égal : non");
	g_k[1].t.prio = 7;
	h_true(!policy_should_preempt(&g_k[0], &g_k[1]), "plus bas : non");
	g_k[0].kflags = KT_IDLE;
	h_true(policy_should_preempt(&g_k[0], &g_k[1]), "inactif : toujours");
}

static void	quantum_limites(void)
{
	uint64_t	start;

	fu_kt_init(&g_k[0], 8);
	g_k[0].t.quantum_left = QUANTUM_NS;
	start = 1000;
	policy_account(&g_k[0], &start, 1000);
	h_eq_u64("durée 0 : rien", g_k[0].t.quantum_left, QUANTUM_NS);
	policy_account(&g_k[0], &start, 1000 + QUANTUM_NS - 1);
	h_eq_u64("q - 1 : reste 1", g_k[0].t.quantum_left, 1);
	policy_account(&g_k[0], &start, 1000 + QUANTUM_NS);
	h_eq_u64("q : épuisé", g_k[0].t.quantum_left, 0);
	g_k[0].t.quantum_left = 5;
	policy_account(&g_k[0], &start, start + 50);
	h_eq_u64("plus que q : épuisé", g_k[0].t.quantum_left, 0);
	h_eq_u64("temps CPU cumulé", g_k[0].t.cpu_time_ns, QUANTUM_NS + 50);
	policy_account(&g_k[0], &start, 10);
	h_eq_u64("temps qui recule : ignoré", start, 1000 + QUANTUM_NS + 50);
}

int	main(void)
{
	h_begin("a06/policy_prio");
	h_run("priorité effective", prio_effective_max);
	h_run("remontée au réveil (limites)", remontee_limites);
	h_run("table de préemption", preemption_table);
	h_run("quantum (limites)", quantum_limites);
	return (h_end());
}
