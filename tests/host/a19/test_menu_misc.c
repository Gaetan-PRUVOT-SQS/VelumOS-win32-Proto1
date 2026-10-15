#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	everything_consumed(void)
{
	t_fakeui	u;
	t_ctl		*m;
	t_ctl		*b;

	fake_begin(&u, 200, 150);
	b = fake_btn(&u);
	m = fake_menu(&u, 20, 20);
	h_true(fake_press(&u.r, VK_TAB, 0), "Tab consomme");
	h_true(fake_press(&u.r, VK_F1, 0), "F1 consomme");
	h_true(fake_press(&u.r, VK_RETURN, 0), "Entree sans selection consomme");
	h_true(fake_char(&u.r, 'x'), "lettre inconnue consommee");
	h_true(u.r.focus == m && b != NULL, "le focus reste au menu");
	h_true(ctl_find(&u.r, 100) == m, "toujours ouvert");
	h_eq_i64("aucune notification", fake_cmd_count(), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/menu_misc");
	h_run("toutes les touches consommees", everything_consumed);
	return (h_end());
}
