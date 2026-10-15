#include "harness.h"
#include "fake.h"

static int	g_destroyed;

static void	count_destroy(t_object *o)
{
	(void)o;
	g_destroyed++;
}

static void	refs_destruction_unique(void)
{
	static const t_objops	ops = {"compteur", count_destroy, NULL};
	t_object				*o;
	t_object				dead;

	fk_reset();
	g_destroyed = 0;
	o = obj_create(OBJ_EVENT, &ops, NULL);
	h_true(o && o->refs == 1, "creation a une reference");
	obj_ref(o);
	obj_ref(o);
	obj_unref(o);
	obj_unref(o);
	h_eq_i64("pas detruit avant la derniere", g_destroyed, 0);
	h_true(obj_tryref(o), "prise faible sur objet vivant");
	obj_unref(o);
	obj_unref(o);
	h_eq_i64("detruit exactement une fois", g_destroyed, 1);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
	dead.refs = 0;
	h_true(!obj_tryref(&dead), "prise faible refusee a zero");
	h_true(!obj_tryref(NULL), "prise faible sur NULL");
	obj_unref(NULL);
}

static void	create_partitions(void)
{
	static const t_objops	ops = {"vide", NULL, NULL};
	t_object				*o;

	fk_reset();
	h_true(!obj_create(OBJ_NONE, &ops, NULL), "type 0 refuse");
	h_true(!obj_create(OBJ_TYPES, &ops, NULL), "type hors borne refuse");
	h_true(!obj_create(OBJ_EVENT, NULL, NULL), "ops NULL refuse");
	g_fk.heap_fail = 1;
	h_true(!obj_create(OBJ_EVENT, &ops, NULL), "echec du tas rendu NULL");
	o = obj_create(OBJ_INPUT, &ops, NULL);
	h_true(o && o->type == OBJ_INPUT, "dernier type valide accepte");
	obj_unref(o);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	wait_signal_timeout(void)
{
	t_object	*ev;

	fk_reset();
	h_eq_i64("evenement manuel", evt_create(1, 0, &ev), 0);
	h_eq_i64("non signale : delai", obj_wait(ev, 100), E_TIMEOUT);
	h_true(g_fk.now >= 1100, "le delai a ete attendu");
	h_eq_i64("set", evt_op(ev, EV_SET), 0);
	h_eq_i64("signale : 0", obj_wait(ev, 0), 0);
	h_eq_i64("manuel reste signale", obj_wait(ev, 0), 0);
	h_eq_i64("attente NULL", obj_wait(NULL, 0), E_INVAL);
	h_eq_i64("op inconnue", evt_op(ev, 3), E_INVAL);
	obj_unref(ev);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/object");
	h_run("refs_destruction_unique", refs_destruction_unique);
	h_run("create_partitions", create_partitions);
	h_run("wait_signal_timeout", wait_signal_timeout);
	return (h_end());
}
