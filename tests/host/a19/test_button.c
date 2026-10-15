#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	click_inside(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = fake_btn(&u);
	h_true(fake_down(&u.r, 15, 15), "appui consomme");
	h_true(u.r.capture == b && u.r.focus == b, "capture et focus");
	h_true((b->state & CTL_ST_PRESSED) != 0, "etat appuye");
	h_eq_i64("rien avant le relachement", fake_cmd_count(), 0);
	h_true(fake_up(&u.r, 15, 15), "relachement consomme");
	h_eq_i64("un seul clic", fake_cmd_find(1, CN_CLICKED), 1);
	h_eq_i64("callback du controle",
		fake_cmd_find(1, CN_CLICKED | FAKE_CB_MARK), 1);
	h_eq_i64("callback avant commande", fake_cmd_code(0),
		CN_CLICKED | FAKE_CB_MARK);
	h_true(!u.r.capture && !(b->state & CTL_ST_PRESSED), "etat relache");
	fake_done(&u);
}

static void	leave_and_return(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = fake_btn(&u);
	fake_down(&u.r, 15, 15);
	fake_move(&u.r, 150, 50);
	h_true(!(b->state & CTL_ST_PRESSED), "sorti : plus appuye");
	h_true(u.r.capture == b, "capture conservee hors du bouton");
	fake_up(&u.r, 150, 50);
	h_eq_i64("relache dehors : pas de clic", fake_cmd_count(), 0);
	fake_down(&u.r, 15, 15);
	fake_move(&u.r, 150, 50);
	fake_move(&u.r, 20, 20);
	h_true((b->state & CTL_ST_PRESSED) != 0, "revenu : de nouveau appuye");
	fake_up(&u.r, 20, 20);
	h_eq_i64("clic apres retour", fake_cmd_find(1, CN_CLICKED), 1);
	fake_done(&u);
}

static void	other_buttons_and_orphans(void)
{
	t_fakeui	u;

	fake_begin(&u, 200, 100);
	fake_btn(&u);
	g_fake_buttons = 1u << BTN_RIGHT;
	h_true(!fake_mouse(&u.r, INP_MOUSE_DOWN, 15, 15), "bouton droit ignore");
	g_fake_buttons = 0;
	h_true(u.r.capture == NULL, "pas de capture");
	fake_mouse(&u.r, INP_MOUSE_UP, 15, 15);
	fake_down(&u.r, 150, 50);
	fake_up(&u.r, 15, 15);
	h_eq_i64("appui dehors, relache dedans", fake_cmd_count(), 0);
	fake_wheel(&u.r, 15, 15, 3);
	h_eq_i64("molette sans effet", fake_cmd_count(), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/button");
	h_run("clic complet : appui, relachement, notifications", click_inside);
	h_run("sortie et retour pendant l'appui", leave_and_return);
	h_run("autres boutons, relachement orphelin", other_buttons_and_orphans);
	return (h_end());
}
