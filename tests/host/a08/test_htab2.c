#include "harness.h"
#include "fake.h"

static void	rights_need(void)
{
	t_process	*p;
	t_object	*o;
	t_handle	h;
	t_hget		g;

	fk_reset();
	p = fk_proc_new(0);
	evt_create(0, 0, &o);
	handle_alloc(p, o, HR_WAIT, &h);
	obj_unref(o);
	handle_get(p, h, OBJ_NONE, &g);
	h_eq_i64("besoin satisfait", handle_need(&g, HR_WAIT), 0);
	h_true(g.obj == o, "reference gardee");
	obj_unref(g.obj);
	handle_get(p, h, OBJ_NONE, &g);
	h_eq_i64("besoin absent", handle_need(&g, HR_SIGNAL), E_PERM);
	h_true(g.obj == NULL, "reference rendue sur refus");
	h_eq_i64("refs", o->refs, 1);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	type_and_args(void)
{
	t_process	*p;
	t_object	*o;
	t_handle	h;
	t_hget		g;

	fk_reset();
	p = fk_proc_new(0);
	evt_create(0, 0, &o);
	handle_alloc(p, o, HR_ALL, &h);
	h_eq_i64("autre type", handle_get(p, h, OBJ_CHANNEL, &g), E_BADF);
	h_eq_i64("processus NULL", handle_get(NULL, h, OBJ_NONE, &g), E_BADF);
	h_eq_i64("sortie NULL", handle_alloc(p, o, HR_ALL, NULL), E_INVAL);
	h_eq_i64("objet NULL", handle_alloc(p, NULL, HR_ALL, &h), E_INVAL);
	h_eq_i64("close NULL", handle_close(NULL, h), E_BADF);
	h_eq_i64("dup sortie NULL", handle_dup(p, h, p, NULL), E_INVAL);
	obj_unref(o);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	exhaustion_growth(void)
{
	t_process	*p;
	t_object	*o;
	t_handle	h;
	int			i;
	int			ok;

	fk_reset();
	p = fk_proc_new(0);
	evt_create(0, 0, &o);
	g_fk.heap_fail = 1;
	h_eq_i64("agrandissement impossible", handle_alloc(p, o, 1, &h), E_NOMEM);
	ok = 0;
	i = 0;
	while (i++ < HANDLE_MAX)
		ok += (handle_alloc(p, o, HR_WAIT, &h) == 0);
	h_eq_i64("4096 handles", ok, HANDLE_MAX);
	h_eq_i64("dernier indice", h & HT_IDX_MASK, HANDLE_MAX - 1);
	h_eq_i64("epuisement", handle_alloc(p, o, HR_WAIT, &h), E_MFILE);
	handle_close(p, (1u << 12) | 7);
	h_eq_i64("place rendue", handle_alloc(p, o, HR_WAIT, &h), 0);
	h_eq_i64("indice libere reutilise", h & HT_IDX_MASK, 7);
	h_eq_i64("refs", o->refs, HANDLE_MAX + 1);
	obj_unref(o);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	retired_and_closed(void)
{
	t_process	*p;
	t_object	*o;
	t_handle	h;
	t_htab		*ht;

	fk_reset();
	p = fk_proc_new(0);
	ht = p->handles;
	evt_create(0, 0, &o);
	handle_alloc(p, o, HR_ALL, &h);
	ht->ents[h & HT_IDX_MASK].gen = HT_GEN_MAX;
	h = ((uint32_t)HT_GEN_MAX << HT_IDX_BITS) | (h & HT_IDX_MASK);
	h_eq_i64("close a la generation max", handle_close(p, h), 0);
	h_eq_i64("case retiree", ht->ents[0].state, HE_RETIRED);
	handle_alloc(p, o, HR_ALL, &h);
	h_true((h & HT_IDX_MASK) != 0, "case retiree jamais reprise");
	htab_close_all(ht);
	h_eq_i64("table fermee", handle_alloc(p, o, HR_ALL, &h), E_CANCELED);
	h_eq_i64("handles fermes", o->refs, 1);
	obj_unref(o);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/htab2");
	h_run("rights_need", rights_need);
	h_run("type_and_args", type_and_args);
	h_run("exhaustion_growth", exhaustion_growth);
	h_run("retired_and_closed", retired_and_closed);
	return (h_end());
}
