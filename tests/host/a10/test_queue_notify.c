#include "harness.h"
#include "th.h"

static void	reveil_hors_verrou(void)
{
	t_reader	*k;
	t_reader	*m;
	t_handle	hk;
	t_handle	hm;

	fake_all_reset();
	hk = th_open(INPUT_KIND_KEYBOARD, &k);
	hm = th_open(INPUT_KIND_MOUSE, &m);
	th_push_n(INP_KEY_DOWN, 1, 3);
	th_push_n(INP_MOUSE_MOVE, 1, 2);
	h_eq_i64("clavier : reveils", fobj_signals(g_fobj.slot[hk - 1]), 3);
	h_eq_i64("souris : reveils", fobj_signals(g_fobj.slot[hm - 1]), 2);
	h_eq_i64("clavier : aucun reveil sous verrou",
		fobj_signals_locked(g_fobj.slot[hk - 1]), 0);
	h_eq_i64("souris : aucun reveil sous verrou",
		fobj_signals_locked(g_fobj.slot[hm - 1]), 0);
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

static void	references_rendues(void)
{
	t_reader	*r;
	t_handle	h;
	t_object	*o;
	uint32_t	avant;

	fake_all_reset();
	h = th_open(INPUT_KIND_ALL, &r);
	o = g_fobj.slot[h - 1];
	avant = o->refs;
	th_push_n(INP_CHAR, 1, 4);
	h_eq_u64("references inchangees apres reveil", o->refs, avant);
	handle_close(proc_current(), h);
	h_eq_i64("aucun objet vivant", g_fobj.live, 0);
}

static void	lecteur_ferme_non_reveille(void)
{
	t_reader	*r;
	t_handle	h;

	fake_all_reset();
	h = th_open(INPUT_KIND_ALL, &r);
	handle_close(proc_current(), h);
	th_push_n(INP_CHAR, 1, 2);
	h_eq_i64("file inchangee sans lecteur", g_fsync.held + g_fsync.errors, 0);
}

int	main(void)
{
	h_begin("a10/queue_notify");
	h_run("reveil apres liberation du verrou", reveil_hors_verrou);
	h_run("references rendues", references_rendues);
	h_run("lecteur ferme", lecteur_ferme_non_reveille);
	return (h_end());
}
