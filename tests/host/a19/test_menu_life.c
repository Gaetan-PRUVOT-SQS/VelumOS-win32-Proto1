#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	second_popup_replaces_first(void)
{
	t_fakeui	u;
	t_ctl		*a;
	t_ctl		*b;

	fake_begin(&u, 200, 150);
	a = fake_menu_id(&u, 10, 10, 100);
	b = fake_menu_id(&u, 30, 30, 101);
	h_true(a != NULL && b != NULL, "deux popups");
	h_true(u.r.capture == b && u.r.focus == b, "le second capture");
	h_true((a->state & CTL_ST_CLOSED) != 0, "le premier est ferme");
	fake_move(&u.r, 1, 1);
	h_true(ctl_find(&u.r, 100) == NULL, "le premier est retire a l'evenement");
	h_true(ctl_find(&u.r, 101) == b, "le second reste");
	fake_done(&u);
}

static void	id_reuse_in_callback(void)
{
	t_fakeui	u;
	t_ctl		*m;
	t_ctl		*again;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 10, 10);
	m->cb = fake_cb_repop;
	m->user = &u;
	fake_keys(&u.r, VK_DOWN, 0, 1);
	fake_press(&u.r, VK_RETURN, 0);
	again = ctl_find(&u.r, 100);
	h_true(again != NULL, "le menu recree dans le callback existe");
	h_true(u.r.capture == again && u.r.focus == again, "et il est actif");
	h_true((again->state & CTL_ST_CLOSED) == 0, "pas ferme");
	fake_keys(&u.r, VK_ESCAPE, 0, 1);
	h_true(ctl_find(&u.r, 100) == NULL, "ferme ensuite");
	fake_done(&u);
}

static void	programmatic_close(void)
{
	t_fakeui	u;
	t_ctl		*b;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	b = fake_btn(&u);
	ctl_focus(&u.r, b);
	m = fake_menu(&u, 20, 20);
	h_eq_i64("resultat avant tout choix", ctl_menu_result(m), 0);
	ctl_menu_close(&u.r, m);
	h_true(ctl_find(&u.r, 100) == NULL, "retire aussitot");
	h_true(u.r.capture == NULL && u.r.focus == b, "focus rendu");
	h_eq_i64("pas de notification", fake_cmd_count(), 0);
	fake_done(&u);
}

static void	popup_while_pressed(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 150);
	b = fake_btn(&u);
	fake_down(&u.r, 15, 15);
	h_true((b->state & CTL_ST_PRESSED) != 0 && u.r.capture == b, "appuye");
	fake_menu(&u, 100, 50);
	h_true(!(b->state & CTL_ST_PRESSED), "le popup relache le bouton");
	fake_up(&u.r, 15, 15);
	h_eq_i64("pas de clic fantome", fake_cmd_find(1, CN_CLICKED), 0);
	h_true(ctl_find(&u.r, 100) != NULL, "le menu reste ouvert");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/menu_life");
	h_run("un second popup ferme le premier", second_popup_replaces_first);
	h_run("id reutilise dans le callback", id_reuse_in_callback);
	h_run("fermeture par programme", programmatic_close);
	h_run("popup pendant un appui", popup_while_pressed);
	return (h_end());
}
