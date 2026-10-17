#include "lt_gal.h"

static const t_lunastate	g_states[6] = {LS_NORMAL, LS_HOT, LS_PRESSED,
	LS_DISABLED, LS_DEFAULT, LS_FOCUSED};
static const char			*g_names[6] = {"Normal", "Survol", "Appuy\xc3\xa9",
	"Inactif", "D\xc3\xa9" "faut", "Focus"};

void	lt_gal_buttons(t_surface *s, int32_t x, int32_t y)
{
	t_lunabtn	b;
	int32_t		i;

	i = 0;
	while (i < 6)
	{
		b.r = lp_rect(x + i * 90, y, 80, 28);
		b.state = g_states[i];
		b.is_default = false;
		b.focus = (i == 5);
		luna_button(s, &b);
		lt_gal_text(s, b.r, g_names[i], luna_button_ink(&b));
		i++;
	}
}

void	lt_gal_checks(t_surface *s, int32_t x, int32_t y)
{
	int32_t	i;

	i = 0;
	while (i < 4)
	{
		luna_checkbox(s, lp_pt(x + i * 60, y), g_states[i], false);
		luna_checkbox(s, lp_pt(x + i * 60 + 20, y), g_states[i], true);
		luna_radio(s, lp_pt(x + i * 60, y + 24), g_states[i], false);
		luna_radio(s, lp_pt(x + i * 60 + 20, y + 24), g_states[i], true);
		i++;
	}
}
