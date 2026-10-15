#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "init.h"

static void	burst(void)
{
	t_restarts	r;
	int			i;
	int			ok;

	memset(&r, 0, sizeof(r));
	ok = 0;
	i = 0;
	while (i < RESTART_MAX)
		ok += restart_allowed(&r, 1000000000ull * (uint64_t)i++);
	h_eq_i64("limite 5 relances en rafale", ok, RESTART_MAX);
	h_eq_i64("6e relance refusee", restart_allowed(&r, 6000000000ull), 0);
	h_eq_i64("limite fenetre - 1 ns : refus",
		restart_allowed(&r, RESTART_WINDOW_NS - 1), 0);
	h_eq_i64("fenetre ecoulee pour la premiere : accepte",
		restart_allowed(&r, RESTART_WINDOW_NS), 1);
	h_eq_i64("puis refus tant que 5 recentes",
		restart_allowed(&r, RESTART_WINDOW_NS + 1), 0);
}

static void	no_clock(void)
{
	t_restarts	r;
	int			i;
	int			ok;

	memset(&r, 0, sizeof(r));
	ok = 0;
	i = 0;
	while (i < 10)
	{
		ok += restart_allowed(&r, 0);
		i++;
	}
	h_eq_i64("sans horloge : 5 relances puis abandon", ok, RESTART_MAX);
}

static void	corrupted(void)
{
	t_restarts	r;

	memset(&r, 0, sizeof(r));
	r.n = RESTART_MAX + 1;
	h_eq_i64("compteur corrompu refuse", restart_allowed(&r, 1), 0);
	r.n = 0;
	r.next = RESTART_MAX;
	h_eq_i64("indice corrompu refuse", restart_allowed(&r, 1), 0);
}

int	main(void)
{
	h_begin("a07/init_relances");
	h_run("rafale", burst);
	h_run("sans horloge", no_clock);
	h_run("etat corrompu", corrupted);
	return (h_end());
}
