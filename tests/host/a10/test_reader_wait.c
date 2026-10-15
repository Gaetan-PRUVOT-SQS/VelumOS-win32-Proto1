#include "harness.h"
#include "th.h"

static void	signals_follow_filters(void)
{
	t_reader	*k;
	t_reader	*m;
	t_handle	hk;
	t_handle	hm;

	fake_all_reset();
	hk = th_open(INPUT_KIND_KEYBOARD, &k);
	hm = th_open(INPUT_KIND_MOUSE, &m);
	th_push_n(INP_KEY_DOWN, 1, 2);
	th_push_n(INP_MOUSE_MOVE, 1, 3);
	th_push_n(INP_WHEEL, 1, 1);
	h_eq_i64("clavier : 2 reveils", fobj_signals(g_fobj.slot[hk - 1]), 2);
	h_eq_i64("souris : 4 reveils", fobj_signals(g_fobj.slot[hm - 1]), 4);
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

static void	ready_predicate(void)
{
	t_reader	*a;
	t_reader	*m;
	uint32_t	codes[4];

	fake_all_reset();
	th_open(INPUT_KIND_ALL, &a);
	th_open(INPUT_KIND_MOUSE, &m);
	h_eq_i64("rien a lire", reader_ready(a), 0);
	th_push_n(INP_KEY_DOWN, 1, 1);
	h_eq_i64("evenement disponible", reader_ready(a), 1);
	th_take_codes(a, codes, 4);
	h_eq_i64("lu : plus pret", reader_ready(a), 0);
	h_eq_i64("souris : reveil superflu possible", reader_ready(m), 1);
	h_eq_i64("souris : rien de livre", th_take_codes(m, codes, 4), 0);
	h_eq_i64("souris : retombe a zero", reader_ready(m), 0);
}

static void	object_signaled_callback(void)
{
	t_reader	*r;
	t_handle	h;
	t_object	*o;

	fake_all_reset();
	h = th_open(INPUT_KIND_ALL, &r);
	o = g_fobj.slot[h - 1];
	h_true(o->ops->signaled(o) == false, "objet non signale");
	th_push_n(INP_CHAR, 1, 1);
	h_true(o->ops->signaled(o) == true, "objet signale");
	h_eq_i64("rights : attente et lecture", g_fobj.rights[h - 1],
		HR_WAIT | HR_READ);
	h_eq_u64("type OBJ_INPUT", o->type, OBJ_INPUT);
}

static void	wipe_when_last_reader_closes(void)
{
	t_reader	*r[2];
	t_handle	h[2];
	uint32_t	codes[4];

	fake_all_reset();
	h[0] = th_open(INPUT_KIND_ALL, &r[0]);
	h[1] = th_open(INPUT_KIND_ALL, &r[1]);
	th_push_n(INP_KEY_DOWN, 0x41, 5);
	handle_close(proc_current(), h[0]);
	h_eq_u64("un lecteur reste : anneau intact", g_inputq.ring[0].code, 0x41);
	h_eq_i64("l'autre lit toujours", th_take_codes(r[1], codes, 4), 4);
	handle_close(proc_current(), h[1]);
	h_eq_u64("dernier ferme : anneau efface", g_inputq.ring[0].code, 0);
	h_eq_u64("type efface", g_inputq.ring[4].type, 0);
	h_eq_i64("aucun objet vivant", g_fobj.live, 0);
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

int	main(void)
{
	h_begin("a10/reader_wait");
	h_run("reveils selon les filtres", signals_follow_filters);
	h_run("predicat de disponibilite", ready_predicate);
	h_run("rappel signaled de l'objet", object_signaled_callback);
	h_run("effacement a la derniere fermeture", wipe_when_last_reader_closes);
	return (h_end());
}
