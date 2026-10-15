#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	down_skip_and_wrap(void)
{
	t_fakeui	u;
	t_ctl		*m;
	const int	down[9] = {0, 1, 2, 4, 5, 6, 8, 0, 1};
	int			i;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	h_eq_i64("rien de selectionne a l'ouverture", fake_msel(m, 0), -1);
	i = 0;
	while (i < 9)
	{
		fake_press(&u.r, VK_DOWN, 0);
		h_eq_i64("Bas saute les separateurs", fake_msel(m, 0), down[i]);
		i++;
	}
	fake_done(&u);
}

static void	up_home_end(void)
{
	t_fakeui	u;
	t_ctl		*m;
	const int	up[4] = {8, 6, 5, 4};
	int			i;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	i = 0;
	while (i < 4)
	{
		fake_press(&u.r, VK_UP, 0);
		h_eq_i64("Haut depuis rien : cyclique", fake_msel(m, 0), up[i]);
		i++;
	}
	fake_press(&u.r, VK_HOME, 0);
	h_eq_i64("Debut", fake_msel(m, 0), 0);
	fake_press(&u.r, VK_END, 0);
	h_eq_i64("Fin", fake_msel(m, 0), 8);
	fake_done(&u);
}

static void	enter_picks_and_restores(void)
{
	t_fakeui	u;
	t_ctl		*b;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	b = fake_btn(&u);
	ctl_focus(&u.r, b);
	m = fake_menu(&u, 20, 20);
	h_true(u.r.capture == m && u.r.focus == m, "menu : capture et focus");
	fake_keys(&u.r, VK_DOWN, 0, 3);
	h_true(fake_press(&u.r, VK_RETURN, 0), "Entree consomme");
	h_eq_i64("element choisi", g_fake_picked, 2);
	h_eq_i64("CN_SELECT", fake_cmd_find(100, CN_SELECT), 1);
	h_true(ctl_find(&u.r, 100) == NULL, "menu retire apres l'evenement");
	h_true(u.r.capture == NULL && u.r.focus == b, "focus rendu");
	h_eq_i64("rien d'alloue en plus", fake_mem_live(), 2);
	fake_done(&u);
}

static void	disabled_and_escape(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	fake_keys(&u.r, VK_DOWN, 0, 4);
	h_eq_i64("desactive : surligne quand meme", fake_msel(m, 0), 4);
	fake_press(&u.r, VK_RETURN, 0);
	fake_press(&u.r, VK_SPACE, 0);
	h_true(ctl_find(&u.r, 100) == m, "desactive : le menu reste ouvert");
	h_eq_i64("desactive : aucune notification", fake_cmd_count(), 0);
	fake_press(&u.r, VK_ESCAPE, 0);
	h_eq_i64("Echap : CN_CLOSED", fake_cmd_find(100, CN_CLOSED), 1);
	h_eq_i64("Echap : pas de CN_SELECT", fake_cmd_find(100, CN_SELECT), 0);
	h_true(ctl_find(&u.r, 100) == NULL && u.r.capture == NULL, "ferme");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/menu_keys");
	h_run("Bas : separateurs sautes, cyclique", down_skip_and_wrap);
	h_run("Haut, Debut, Fin", up_home_end);
	h_run("Entree : choix, focus rendu, menu retire", enter_picks_and_restores);
	h_run("element desactive inerte, Echap", disabled_and_escape);
	return (h_end());
}
