#include "harness.h"
#include "lt_contrast.h"
#include "velum/theme.h"

static void	ink_is(t_lunastate st, bool def, t_color want, const char *msg)
{
	t_lunabtn	b;

	b.r = lp_rect(0, 0, 10, 10);
	b.state = st;
	b.is_default = def;
	b.focus = false;
	h_eq_u64(msg, luna_button_ink(&b), want);
}

static void	table_encre(void)
{
	luna_theme_set(LUNA_THEME_LIGHT);
	ink_is(LS_NORMAL, false, VT_INK, "clair, normal : encre");
	ink_is(LS_HOT, false, VT_INK, "clair, survol : encre");
	ink_is(LS_PRESSED, false, VT_INK, "clair, appui : encre");
	ink_is(LS_DISABLED, false, VT_DIM, "clair, inactif : discret");
	ink_is(LS_NORMAL, true, LC_WHITE, "principal : blanc");
	ink_is(LS_DEFAULT, false, LC_WHITE, "etat defaut : blanc");
	ink_is(LS_DISABLED, true, VT_DIM, "principal inactif : discret");
	h_eq_u64("sans bouton : encre", luna_button_ink(NULL), VT_INK);
	luna_theme_set(LUNA_THEME_DARK);
	ink_is(LS_NORMAL, false, VT_INK_D, "sombre, normal : encre");
	ink_is(LS_DISABLED, false, VT_DIM_D, "sombre, inactif : discret");
	ink_is(LS_NORMAL, true, LC_WHITE, "sombre, principal : blanc");
	luna_theme_set(LUNA_THEME_LIGHT);
}

int	main(void)
{
	h_begin("a17/ctrl_encre");
	h_run("table de decision : encre du bouton", table_encre);
	return (h_end());
}
