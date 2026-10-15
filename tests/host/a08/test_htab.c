#include "harness.h"
#include "fake.h"

static void	generation(void)
{
	t_process	*p;
	t_object	*o;
	t_handle	h1;
	t_handle	h2;

	fk_reset();
	p = fk_proc_new(0);
	evt_create(0, 0, &o);
	h_eq_i64("alloc", handle_alloc(p, o, HR_ALL, &h1), 0);
	h_true(h1 != HANDLE_INVALID, "handle non nul");
	h_eq_i64("close", handle_close(p, h1), 0);
	h_eq_i64("double fermeture", handle_close(p, h1), E_BADF);
	h_eq_i64("realloc", handle_alloc(p, o, HR_ALL, &h2), 0);
	h_eq_i64("meme indice", h1 & HT_IDX_MASK, h2 & HT_IDX_MASK);
	h_true(h1 != h2, "generation differente");
	h_eq_i64("ancien handle refuse", handle_close(p, h1), E_BADF);
	obj_unref(o);
	h_eq_i64("dernier handle detruit", handle_close(p, h2), 0);
	h_eq_i64("objet detruit", g_fk.heap_live[HEAP_OBJECT], 3);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	invalid_values(void)
{
	t_process	*p;
	t_object	*o;
	t_handle	h;
	t_hget		g;

	fk_reset();
	p = fk_proc_new(0);
	evt_create(0, 0, &o);
	handle_alloc(p, o, HR_ALL, &h);
	h_eq_i64("handle 0", handle_get(p, HANDLE_INVALID, OBJ_NONE, &g), E_BADF);
	h_eq_i64("indice 4095 hors table", handle_get(p, 4095 | (1u << 12),
			OBJ_NONE, &g), E_BADF);
	h_eq_i64("generation 0", handle_get(p, h & HT_IDX_MASK, OBJ_NONE, &g),
		E_BADF);
	h_eq_i64("generation suivante", handle_get(p, h + (1u << 12), OBJ_NONE,
			&g), E_BADF);
	h_eq_i64("valeur max", handle_get(p, 0xffffffffu, OBJ_NONE, &g), E_BADF);
	obj_unref(o);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	get_refs(void)
{
	t_process	*p;
	t_object	*o;
	t_handle	h;
	t_hget		g;

	fk_reset();
	p = fk_proc_new(0);
	evt_create(0, 0, &o);
	handle_alloc(p, o, HR_WAIT, &h);
	h_eq_i64("alloc prend sa reference", o->refs, 2);
	h_eq_i64("get", handle_get(p, h, OBJ_EVENT, &g), 0);
	h_true(g.obj == o && o->refs == 3, "get rend une reference");
	h_eq_i64("droits rendus", g.rights, HR_WAIT);
	obj_unref(g.obj);
	obj_unref(o);
	h_eq_i64("type quelconque", handle_get(p, h, OBJ_NONE, &g), 0);
	obj_unref(g.obj);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	rights_dup(void)
{
	t_process	*p;
	t_object	*o;
	t_handle	h;
	t_handle	d;

	fk_reset();
	p = fk_proc_new(0);
	evt_create(0, 0, &o);
	h_eq_i64("bits inconnus", handle_alloc(p, o, 0x200, &h), E_INVAL);
	handle_alloc(p, o, HR_WAIT | HR_DUP, &h);
	obj_unref(o);
	h_eq_i64("dup elargi", handle_dup_as(p, h, HR_SIGNAL, &d), E_PERM);
	h_eq_i64("dup bits inconnus", handle_dup_as(p, h, 0x400, &d), E_INVAL);
	h_eq_i64("dup reduit", handle_dup_as(p, h, HR_WAIT, &d), 0);
	h_eq_i64("dup sans HR_DUP", handle_dup_as(p, d, HR_WAIT, &h), E_PERM);
	h_eq_i64("dup inter sans HR_DUP", handle_dup(p, d, p, &h), E_PERM);
	h_eq_i64("dup inter", handle_dup(p, h, p, &d), 0);
	h_eq_i64("refs : 3 handles", o->refs, 3);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/htab");
	h_run("generation", generation);
	h_run("invalid_values", invalid_values);
	h_run("get_refs", get_refs);
	h_run("rights_dup", rights_dup);
	return (h_end());
}
