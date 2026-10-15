#include "harness.h"
#include "fake.h"

static void	any_index_consume(void)
{
	t_object	*e[3];

	fk_reset();
	evt_create(0, 0, &e[0]);
	evt_create(0, 0, &e[1]);
	evt_create(1, 0, &e[2]);
	h_eq_i64("rien de signale", wait_objects(e, 3, WAIT_ANY, 0), E_TIMEOUT);
	evt_op(e[1], EV_SET);
	h_eq_i64("indice du signale", wait_objects(e, 3, WAIT_ANY, 0), 1);
	h_eq_i64("auto consomme", wait_objects(e, 3, WAIT_ANY, 0), E_TIMEOUT);
	evt_op(e[2], EV_SET);
	evt_op(e[0], EV_SET);
	h_eq_i64("premier indice", wait_objects(e, 3, WAIT_ANY, 0), 0);
	h_eq_i64("manuel ensuite", wait_objects(e, 3, WAIT_ANY, 0), 2);
	h_eq_i64("manuel non consomme", wait_objects(e, 3, WAIT_ANY, 0), 2);
	evt_op(e[2], EV_RESET);
	h_eq_i64("reset", wait_objects(e, 3, WAIT_ANY, 0), E_TIMEOUT);
	obj_unref(e[0]);
	obj_unref(e[1]);
	obj_unref(e[2]);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	all_or_nothing(void)
{
	t_object	*e[2];

	fk_reset();
	evt_create(0, 1, &e[0]);
	evt_create(1, 0, &e[1]);
	h_eq_i64("un seul pret", wait_objects(e, 2, WAIT_ALL, 0), E_TIMEOUT);
	h_true(sig_signaled(e[0]), "rien consomme sur echec");
	evt_op(e[1], EV_SET);
	h_eq_i64("tous prets", wait_objects(e, 2, WAIT_ALL, 0), 0);
	h_true(!sig_signaled(e[0]), "auto consomme");
	h_true(sig_signaled(e[1]), "manuel garde");
	h_eq_i64("auto deja pris", wait_objects(e, 2, WAIT_ALL, 0), E_TIMEOUT);
	obj_unref(e[0]);
	obj_unref(e[1]);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	limits(void)
{
	t_object	*e[WAIT_MAX + 1];
	int			i;

	fk_reset();
	i = 0;
	while (i < WAIT_MAX + 1)
		evt_create(1, 0, &e[i++]);
	evt_op(e[WAIT_MAX - 1], EV_SET);
	h_eq_i64("n = 0", wait_objects(e, 0, WAIT_ANY, 0), E_INVAL);
	h_eq_i64("n = 65", wait_objects(e, 65, WAIT_ANY, 0), E_INVAL);
	h_eq_i64("mode 2", wait_objects(e, 1, 2, 0), E_INVAL);
	h_eq_i64("liste NULL", wait_objects(NULL, 1, WAIT_ANY, 0), E_INVAL);
	h_eq_i64("n = 64", wait_objects(e, 64, WAIT_ANY, 0), WAIT_MAX - 1);
	g_fk.heap_fail = 1;
	h_eq_i64("tas plein", wait_objects(e, 1, WAIT_ANY, 0), E_NOMEM);
	h_eq_i64("delai 0 sans dormir", g_fk.waits, 0);
	h_eq_i64("infini sans signal", obj_wait(e[0], TIMEOUT_INF), E_CANCELED);
	h_eq_i64("delai enorme sature", obj_wait(e[0], TIMEOUT_INF - 1),
		E_CANCELED);
	i = 0;
	while (i < WAIT_MAX + 1)
		obj_unref(e[i++]);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/wait");
	h_run("any_index_consume", any_index_consume);
	h_run("all_or_nothing", all_or_nothing);
	h_run("limits", limits);
	return (h_end());
}
