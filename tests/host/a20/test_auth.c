#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "auth.h"

static const uint32_t	g_want[] = {1, 2, 4, 8, 16, 30, 30, 30};

static void	load_set(t_accounts *set)
{
	static const char	text[] = "Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2
		":0\nAlice:12000:" V_SALT_B ":" V_HB1 V_HB2 ":0\nBob:12000:" V_SALT_B
		":" V_HB1 V_HB2 ":1\n";

	h_eq_i64("trois comptes", acc_parse_file(text, sizeof(text) - 1, set), 3);
}

static void	auth_table_de_decision(void)
{
	t_accounts	set;
	t_limiters	lim;
	t_authreq	rq;
	uint32_t	w;

	load_set(&set);
	lim_init(&lim);
	rq = (t_authreq){&set, &lim, 0, 50 * NS_SEC};
	h_eq_i64("vide accepte", auth_attempt(&rq, "", 0, &w), AUTH_OK);
	rq.index = 1;
	h_eq_i64("mauvais", auth_attempt(&rq, "nope", 4, &w), AUTH_BAD);
	h_eq_i64("bloque meme avec le bon mot de passe",
		auth_attempt(&rq, "motdepasse", 10, &w), AUTH_WAIT);
	h_eq_u64("attente annoncee", w, 1);
	rq.now_ns += NS_SEC;
	h_eq_i64("bon mot de passe apres le delai",
		auth_attempt(&rq, "motdepasse", 10, &w), AUTH_OK);
	h_eq_u64("compteur remis a zero", lim.slot[1].failures, 0);
	rq.index = 2;
	h_eq_i64("compte desactive", auth_attempt(&rq, "motdepasse", 10, &w),
		AUTH_DENIED);
	h_eq_u64("pas de delai pour un refus", lim.slot[2].failures, 0);
	rq.index = 3;
	h_eq_i64("indice hors liste", auth_attempt(&rq, "", 0, &w), AUTH_DENIED);
}

static void	auth_escalade_jusqu_a_trente_secondes(void)
{
	t_accounts	set;
	t_limiters	lim;
	t_authreq	rq;
	uint32_t	w;
	uint32_t	i;

	load_set(&set);
	lim_init(&lim);
	rq = (t_authreq){&set, &lim, 1, 1000 * NS_SEC};
	i = 0;
	while (i < sizeof(g_want) / sizeof(g_want[0]))
	{
		h_eq_i64("echec", auth_attempt(&rq, "faux", 4, &w), AUTH_BAD);
		h_eq_i64("attente", auth_attempt(&rq, "faux", 4, &w), AUTH_WAIT);
		h_eq_u64("duree", w, g_want[i]);
		rq.now_ns += g_want[i] * NS_SEC;
		i++;
	}
}

static void	auth_independance_et_entrees_hostiles(void)
{
	t_accounts	set;
	t_limiters	lim;
	t_authreq	rq;
	uint32_t	w;
	static char	big[2048];

	load_set(&set);
	lim_init(&lim);
	rq = (t_authreq){&set, &lim, 1, 0};
	memset(big, 'a', sizeof(big));
	h_eq_i64("trop long", auth_attempt(&rq, big, sizeof(big), &w), AUTH_BAD);
	rq.index = 0;
	h_eq_i64("autre compte libre", auth_attempt(&rq, "", 0, &w), AUTH_OK);
	h_eq_i64("vide refuse pour Alice", auth_attempt(&(t_authreq){&set, &lim, 1,
			NS_SEC}, "", 0, &w), AUTH_BAD);
	h_eq_i64("requete sans liste", auth_attempt(&(t_authreq){NULL, &lim, 0, 0},
			"", 0, &w), AUTH_DENIED);
	h_eq_i64("requete sans limiteur", auth_attempt(&(t_authreq){&set, NULL, 0,
			0}, "", 0, &w), AUTH_DENIED);
}

int	main(void)
{
	h_begin("a20/auth");
	h_run("auth: table de decision", auth_table_de_decision);
	h_run("auth: escalade 1 a 30 s", auth_escalade_jusqu_a_trente_secondes);
	h_run("auth: independance, entrees hostiles",
		auth_independance_et_entrees_hostiles);
	return (h_end());
}
