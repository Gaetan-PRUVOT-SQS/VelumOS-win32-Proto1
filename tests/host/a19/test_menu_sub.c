#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	submenu_keys(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	fake_keys(&u.r, VK_DOWN, 0, 6);
	fake_press(&u.r, VK_RIGHT, 0);
	h_true(fake_mdepth(m) == 2 && fake_msel(m, 1) == 0, "Droite : sous-menu");
	fake_keys(&u.r, VK_DOWN, 0, 2);
	h_eq_i64("Bas saute le separateur", fake_msel(m, 1), 3);
	fake_press(&u.r, VK_RIGHT, 0);
	h_eq_i64("troisieme niveau", fake_mdepth(m), 3);
	fake_press(&u.r, VK_LEFT, 0);
	fake_press(&u.r, VK_LEFT, 0);
	h_eq_i64("Gauche x2 : retour au premier niveau", fake_mdepth(m), 1);
	fake_press(&u.r, VK_LEFT, 0);
	h_true(ctl_find(&u.r, 100) == m && fake_mdepth(m) == 1, "Gauche seul");
	fake_press(&u.r, VK_RIGHT, 0);
	fake_press(&u.r, VK_ESCAPE, 0);
	h_true(ctl_find(&u.r, 100) == m, "Echap : le menu reste");
	h_eq_i64("Echap : un niveau en moins", fake_mdepth(m), 1);
	fake_press(&u.r, VK_ESCAPE, 0);
	h_eq_i64("Echap ferme le menu", fake_cmd_find(100, CN_CLOSED), 1);
	fake_done(&u);
}

static void	enter_in_submenu(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	fake_keys(&u.r, VK_DOWN, 0, 6);
	fake_press(&u.r, VK_RETURN, 0);
	h_true(ctl_find(&u.r, 100) == m && fake_mdepth(m) == 2, "Entree : parent");
	h_eq_i64("rien choisi", fake_cmd_find(100, CN_SELECT), 0);
	fake_press(&u.r, VK_DOWN, 0);
	fake_press(&u.r, VK_RETURN, 0);
	h_eq_i64("Beta choisi", g_fake_picked, 52);
	h_true(ctl_find(&u.r, 100) == NULL, "tout est ferme");
	m = fake_menu(&u, 20, 20);
	fake_keys(&u.r, VK_DOWN, 0, 6);
	fake_keys(&u.r, VK_RETURN, 0, 1);
	fake_keys(&u.r, VK_DOWN, 0, 2);
	fake_keys(&u.r, VK_SPACE, 0, 2);
	h_eq_i64("Delta choisi au troisieme niveau", g_fake_picked, 531);
	fake_done(&u);
}

static void	mnemonics(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	fake_char(&u.r, 'n');
	h_eq_i64("n : premier", fake_msel(m, 0), 0);
	fake_char(&u.r, 'n');
	h_eq_i64("n : suivant", fake_msel(m, 0), 1);
	fake_char(&u.r, 'N');
	h_eq_i64("N : cyclique", fake_msel(m, 0), 0);
	h_true(ctl_find(&u.r, 100) == m, "doublons : pas d'activation");
	fake_char(&u.r, 'f');
	fake_char(&u.r, 'z');
	h_eq_i64("desactive et inconnu : ignores", fake_msel(m, 0), 0);
	fake_char(&u.r, 'p');
	h_eq_i64("p : ouvre le sous-menu", fake_mdepth(m), 2);
	fake_char(&u.r, 'b');
	h_eq_i64("b : Beta unique, choisi", g_fake_picked, 52);
	fake_done(&u);
}

static void	mnemonic_unique(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	fake_char(&u.r, 'O');
	h_eq_i64("O : Ouvrir choisi aussitot", g_fake_picked, 2);
	h_true(ctl_find(&u.r, 100) == NULL && m != NULL, "ferme");
	fake_menu(&u, 20, 20);
	fake_char(&u.r, 'e');
	h_eq_i64("e : element coche aussi activable", g_fake_picked, 4);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/menu_sub");
	h_run("sous-menus : Droite, Gauche, Echap", submenu_keys);
	h_run("Entree dans les sous-menus", enter_in_submenu);
	h_run("mnemoniques : doublons, desactive, sous-menu", mnemonics);
	h_run("mnemonique unique : choix immediat", mnemonic_unique);
	return (h_end());
}
