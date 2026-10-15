#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	default_and_cancel(void)
{
	t_fakeui	u;
	t_ctl		*ok;
	t_ctl		*k;

	fake_begin(&u, 200, 100);
	ok = fk_c(&u, CT_BUTTON, CTL_ID_OK, rect_make(0, 0, 60, 20));
	ctl_set_flag(&u.r, ok, CTL_DEFAULT, true);
	fk_c(&u, CT_BUTTON, CTL_ID_CANCEL, rect_make(80, 0, 60, 20));
	k = fk_c(&u, CT_CHECK, 9, rect_make(10, 40, 80, 14));
	ctl_focus(&u.r, k);
	h_true(fake_press(&u.r, VK_RETURN, 0), "Entree consomme");
	h_eq_i64("bouton par defaut", fake_cmd_find(CTL_ID_OK, CN_CLICKED), 1);
	h_true(fake_press(&u.r, VK_ESCAPE, 0), "Echap consomme");
	h_eq_i64("annulation", fake_cmd_find(CTL_ID_CANCEL, CN_CLICKED), 1);
	h_true(!fake_press(&u.r, VK_RETURN, INPM_REPEAT), "repetition ignoree");
	ctl_set_flag(&u.r, ok, CTL_ENABLED, false);
	h_true(!fake_press(&u.r, VK_RETURN, 0), "defaut desactive : non consomme");
	ctl_set_flag(&u.r, ctl_find(&u.r, CTL_ID_CANCEL), CTL_VISIBLE, false);
	h_true(!fake_press(&u.r, VK_ESCAPE, 0), "annulation masquee");
	fake_done(&u);
}

static void	enter_in_edit(void)
{
	t_fakeui	u;
	t_ctl		*e;

	fake_begin(&u, 200, 100);
	e = fk_c(&u, CT_EDIT, 4, rect_make(10, 40, 80, 20));
	fk_c(&u, CT_BUTTON, CTL_ID_OK, rect_make(0, 0, 60, 20));
	ctl_set_flag(&u.r, ctl_find(&u.r, CTL_ID_OK), CTL_DEFAULT, true);
	fk_c(&u, CT_BUTTON, CTL_ID_CANCEL, rect_make(80, 0, 60, 20));
	ctl_focus(&u.r, e);
	h_true(fake_press(&u.r, VK_RETURN, 0), "Entree dans l'edit consomme");
	h_eq_i64("CN_ENTER", fake_cmd_find(4, CN_ENTER), 1);
	h_eq_i64("pas de defaut", fake_cmd_find(CTL_ID_OK, CN_CLICKED), 0);
	h_true(fake_press(&u.r, VK_ESCAPE, 0), "Echap dans l'edit : annulation");
	h_eq_i64("annulation", fake_cmd_find(CTL_ID_CANCEL, CN_CLICKED), 1);
	fake_done(&u);
}

static void	removal_inside_callback(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = fake_btn(&u);
	b->cb = fake_cb_kill;
	b->user = &u.r;
	ctl_focus(&u.r, b);
	fake_press(&u.r, VK_SPACE, 0);
	h_true(ctl_find(&u.r, 1) == NULL, "controle retire par son callback");
	h_eq_i64("pas de commande sur un controle disparu", fake_cmd_count(), 0);
	h_true(u.r.focus == NULL, "focus remis a zero");
	fake_done(&u);
}

static void	destroy_inside_callback(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = fake_btn(&u);
	b->cb = fake_cb_destroy;
	b->user = &u.r;
	h_true(fake_down(&u.r, 15, 15), "appui");
	fake_up(&u.r, 15, 15);
	h_true(u.r.root == NULL, "racine detruite par le callback");
	h_true(!fake_down(&u.r, 15, 15), "racine absente : evenements ignores");
	h_eq_i64("rien d'alloue", fake_mem_live(), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/button_default");
	h_run("Entree : bouton par defaut, Echap : annulation", default_and_cancel);
	h_run("Entree dans un champ : CN_ENTER seulement", enter_in_edit);
	h_run("retrait du controle dans son callback", removal_inside_callback);
	h_run("destruction de la racine dans un callback", destroy_inside_callback);
	return (h_end());
}
