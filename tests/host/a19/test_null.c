#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	null_arguments(void)
{
	t_ctlspec	sp;
	t_wmmouse	m;
	t_inpevent	ev;

	memset(&m, 0, sizeof(m));
	memset(&ev, 0, sizeof(ev));
	sp = fake_spec(CT_LABEL, 1, rect_make(0, 0, 5, 5), "");
	h_true(ctl_add(NULL, NULL, &sp) == NULL, "ctl_add sans racine");
	ctl_remove(NULL, NULL);
	ctl_set_text(NULL, NULL, "x");
	ctl_set_flag(NULL, NULL, CTL_VISIBLE, true);
	ctl_invalidate(NULL, NULL);
	ctl_focus(NULL, NULL);
	ctl_progress_set(NULL, NULL, 5);
	h_true(ctl_find(NULL, 1) == NULL, "ctl_find sans racine");
	h_true(!ctl_mouse(NULL, &m), "ctl_mouse sans racine");
	h_true(!ctl_key(NULL, &ev), "ctl_key sans racine");
	h_eq_i64("ctl_paint sans racine", ctl_paint(NULL).w, 0);
	h_eq_i64("set_rect sans racine", ctl_set_rect(NULL, NULL, sp.rect),
		E_INVAL);
}

static void	null_after_init(void)
{
	t_fakeui	u;
	t_ctl		*c;

	fake_begin(&u, 50, 50);
	c = fake_add(&u, fake_spec(CT_LIST, 1, rect_make(0, 0, 30, 30), ""));
	h_true(ctl_add(&u.r, NULL, NULL) == NULL, "spec NULL");
	h_true(!ctl_mouse(&u.r, NULL), "evenement souris NULL");
	h_true(!ctl_key(&u.r, NULL), "evenement clavier NULL");
	h_eq_i64("list_add item NULL", ctl_list_add(&u.r, c, NULL), E_INVAL);
	h_eq_i64("list_add sur NULL", ctl_list_add(&u.r, NULL, "x"), E_INVAL);
	h_eq_i64("list_count NULL", ctl_list_count(NULL), E_INVAL);
	h_eq_i64("list_selected NULL", ctl_list_selected(NULL), -1);
	h_eq_i64("list_text NULL", ctl_list_text(NULL, 0, NULL, 0), E_INVAL);
	h_eq_i64("scroll_pos NULL", ctl_scroll_pos(NULL), 0);
	h_eq_i64("menu_result NULL", ctl_menu_result(NULL), 0);
	h_true(ctl_menu_popup(&u.r, NULL) == NULL, "popup sans spec");
	ctl_menu_close(&u.r, NULL);
	fake_done(&u);
}

static void	wrong_type_calls(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 50, 50);
	b = fake_add(&u, fake_spec(CT_BUTTON, 1, rect_make(0, 0, 30, 30), "b"));
	h_eq_i64("list_add sur un bouton", ctl_list_add(&u.r, b, "x"), E_INVAL);
	h_eq_i64("edit_set_limit sur un bouton", ctl_edit_set_limit(&u.r, b, 3),
		E_INVAL);
	h_eq_i64("list_count sur un bouton", ctl_list_count(b), E_INVAL);
	ctl_progress_set(&u.r, b, 50);
	ctl_scroll_set(&u.r, b, 10, 2);
	ctl_scroll_move(&u.r, b, 3);
	h_eq_i64("scroll_pos sur un bouton", ctl_scroll_pos(b), 0);
	ctl_menu_close(&u.r, b);
	h_true(ctl_find(&u.r, 1) == b, "le bouton n'a pas ete touche");
	h_eq_i64("list_select sur un bouton", ctl_list_select(&u.r, b, 0), E_INVAL);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/null");
	h_run("arguments NULL sans racine", null_arguments);
	h_run("arguments NULL apres init", null_after_init);
	h_run("appels sur un controle du mauvais type", wrong_type_calls);
	return (h_end());
}
