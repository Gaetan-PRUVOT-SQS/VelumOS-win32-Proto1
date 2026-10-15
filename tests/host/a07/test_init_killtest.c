#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "init.h"

static int	asked(const char *a0, const char *a1, const char *a2)
{
	char	*argv[4];
	int		argc;

	argv[0] = (char *)a0;
	argv[1] = (char *)a1;
	argv[2] = (char *)a2;
	argv[3] = NULL;
	argc = 0;
	while (argc < 3 && argv[argc])
		argc++;
	return (killtest_requested(argc, argv));
}

static void	option(void)
{
	char	*seul[1];

	seul[0] = KILLTEST_ARG;
	h_eq_i64("argument exact : armé", asked("init", KILLTEST_ARG, NULL), 1);
	h_eq_i64("argument en seconde place : armé",
		asked("init", "init.autre", KILLTEST_ARG), 1);
	h_eq_i64("sans argument : inerte", asked("/system/bin/init", 0, 0), 0);
	h_eq_i64("argv[0] ne compte pas", asked(KILLTEST_ARG, NULL, NULL), 0);
	h_eq_i64("ancien chemin marqueur : inerte",
		asked("/system/bin/mort-winsrv/../init", NULL, NULL), 0);
	h_eq_i64("préfixe : inerte", asked("init", "init.mort-winsr", 0), 0);
	h_eq_i64("suffixe : inerte", asked("init", "init.mort-winsrv2", 0), 0);
	h_eq_i64("chaîne vide : inerte", asked("init", "", NULL), 0);
	h_eq_i64("argv absent : inerte", killtest_requested(2, NULL), 0);
	h_eq_i64("argc nul : inerte", killtest_requested(0, seul), 0);
	h_eq_i64("argc négatif : inerte", killtest_requested(-1, seul), 0);
}

static void	inerte(void)
{
	t_killtest	kt;

	memset(&kt, 0, sizeof(kt));
	kt.state = KT_OFF;
	h_eq_i64("éteint, session prête : rien",
		killtest_step(&kt, 1, 10 * KILLTEST_GRACE_NS), 0);
	h_eq_i64("éteint : reste éteint", kt.state, KT_OFF);
	kt.state = KT_DONE;
	h_eq_i64("déjà tiré : jamais deux fois",
		killtest_step(&kt, 1, 10 * KILLTEST_GRACE_NS), 0);
	h_eq_i64("déjà tiré : reste fini", kt.state, KT_DONE);
	kt.state = 77;
	h_eq_i64("état inconnu : rien", killtest_step(&kt, 1, 1), 0);
	kt.state = KT_ARMED;
	killtest_step(&kt, 1, 1000);
	h_eq_i64("horloge qui recule : grâce repart",
		killtest_step(&kt, 1, 10), 0);
	h_eq_i64("horloge qui recule : repère remis", kt.seen_ns, 10);
	h_eq_i64("sans horloge : jamais de tir", killtest_step(&kt, 1, 10), 0);
}

static void	transitions(void)
{
	t_killtest	kt;

	memset(&kt, 0, sizeof(kt));
	kt.state = KT_ARMED;
	h_eq_i64("armé sans session : attend", killtest_step(&kt, 0, 5), 0);
	h_eq_i64("armé sans session : reste armé", kt.state, KT_ARMED);
	h_eq_i64("session vue : délai de grâce", killtest_step(&kt, 1, 100), 0);
	h_eq_i64("session vue : état grâce", kt.state, KT_GRACE);
	h_eq_i64("grâce - 1 ns : attend",
		killtest_step(&kt, 1, 100 + KILLTEST_GRACE_NS - 1), 0);
	h_eq_i64("session perdue pendant la grâce : réarmé",
		killtest_step(&kt, 0, 200), 0);
	h_eq_i64("réarmé", kt.state, KT_ARMED);
	h_eq_i64("session revue : grâce repart", killtest_step(&kt, 1, 300), 0);
	h_eq_i64("grâce écoulée pile : tire une fois",
		killtest_step(&kt, 1, 300 + KILLTEST_GRACE_NS), 1);
	h_eq_i64("tiré : état fini", kt.state, KT_DONE);
	h_eq_i64("après le tir : plus rien",
		killtest_step(&kt, 1, 300 + 2 * KILLTEST_GRACE_NS), 0);
}

int	main(void)
{
	h_begin("a07/init_essai_mort_winsrv");
	h_run("option", option);
	h_run("inerte", inerte);
	h_run("transitions", transitions);
	return (h_end());
}
