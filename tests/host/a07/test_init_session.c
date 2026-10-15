#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "init.h"

static void	echeance(void)
{
	t_killtest	kt;
	uint32_t	i;
	int			fired;

	memset(&kt, 0, sizeof(kt));
	kt.state = KT_ARMED;
	h_true(killtest_timeout(&kt) == KILLTEST_POLL_NS, "armé : tic demandé");
	fired = 0;
	i = 0;
	while (i < KILLTEST_TICKS_MAX)
		fired += killtest_step(&kt, 0, 1000 * (uint64_t)i++);
	h_eq_i64("session jamais vue : aucun tir", fired, 0);
	h_eq_i64("dernier tic permis : encore armé", kt.state, KT_ARMED);
	h_eq_i64("échéance : pas de tir", killtest_step(&kt, 0, 1), 0);
	h_eq_i64("échéance : essai abandonné", kt.state, KT_EXPIRED);
	h_true(killtest_timeout(&kt) == TIMEOUT_INF,
		"abandonné : plus aucun tic demandé");
	h_eq_i64("abandonné, session vue trop tard : rien",
		killtest_step(&kt, 1, 10 * KILLTEST_GRACE_NS), 0);
	h_eq_i64("abandonné : reste abandonné", kt.state, KT_EXPIRED);
	h_eq_i64("abandonné : compteur figé", kt.ticks, KILLTEST_TICKS_MAX);
}

static void	sans_horloge(void)
{
	t_killtest	kt;
	uint32_t	i;
	int			fired;

	memset(&kt, 0, sizeof(kt));
	kt.state = KT_ARMED;
	fired = 0;
	i = 0;
	while (i++ < KILLTEST_TICKS_MAX)
		fired += killtest_step(&kt, 1, 0);
	h_eq_i64("session vue, horloge figée : aucun tir", fired, 0);
	h_eq_i64("horloge figée : en grâce", kt.state, KT_GRACE);
	h_true(killtest_timeout(&kt) == KILLTEST_POLL_NS, "grâce : tic demandé");
	h_eq_i64("horloge figée : pas de tir", killtest_step(&kt, 1, 0), 0);
	h_eq_i64("horloge figée : abandon à l'échéance", kt.state, KT_EXPIRED);
	kt.state = KT_OFF;
	h_true(killtest_timeout(&kt) == TIMEOUT_INF, "éteint : aucun tic");
	kt.state = KT_DONE;
	h_true(killtest_timeout(&kt) == TIMEOUT_INF, "tiré : aucun tic");
	kt.state = 77;
	h_true(killtest_timeout(&kt) == TIMEOUT_INF, "état inconnu : aucun tic");
}

static int	seen_at(t_procinfo *ps, uint32_t at, uint32_t n, const char *nm)
{
	memset(ps, 0, sizeof(t_procinfo) * INIT_PROCS_MAX);
	memset(ps[at].name, 'x', sizeof(ps[at].name));
	memcpy(ps[at].name, nm, strlen(nm) + 1);
	return (procs_have(ps, n, "shell"));
}

static void	table(void)
{
	t_procinfo	ps[INIT_PROCS_MAX];

	h_eq_i64("table absente", procs_have(NULL, 4, "shell"), 0);
	h_eq_i64("table vide", seen_at(ps, 0, 0, "shell"), 0);
	h_eq_i64("1er emplacement : vu", seen_at(ps, 0, 1, "shell"), 1);
	h_eq_i64("32e emplacement : vu", seen_at(ps, 31, 32, "shell"), 1);
	h_eq_i64("33e emplacement : vu", seen_at(ps, 32, 33, "shell"), 1);
	h_eq_i64("256e emplacement : vu", seen_at(ps, 255, 256, "shell"), 1);
	h_eq_i64("au-delà du compte rendu : pas vu",
		seen_at(ps, 255, 255, "shell"), 0);
	h_eq_i64("compte supérieur à la table : borné",
		seen_at(ps, 255, 100000, "shell"), 1);
	h_eq_i64("nom plus long : pas vu", seen_at(ps, 3, 8, "shells"), 0);
	h_eq_i64("nom plus court : pas vu", seen_at(ps, 3, 8, "shel"), 0);
	memset(ps[3].name, 's', sizeof(ps[3].name));
	h_eq_i64("nom sans NUL : pas vu, lecture bornée",
		procs_have(ps, 8, "shell"), 0);
	h_eq_i64("capacité alignée sur la table du noyau", INIT_PROCS_MAX, 256);
}

int	main(void)
{
	h_begin("a07/init_session_vue");
	h_run("echeance", echeance);
	h_run("sans horloge", sans_horloge);
	h_run("table", table);
	return (h_end());
}
