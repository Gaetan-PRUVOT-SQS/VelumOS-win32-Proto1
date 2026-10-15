#include "lt_ctl_gal.h"

static const t_lunastate	g_st[5] = {LS_NORMAL, LS_HOT, LS_PRESSED,
	LS_DISABLED, LS_FOCUSED};
static const char			*g_nm[5] = {"Normal", "Survol", "Appuy\xc3\xa9",
	"Inactif", "Focus"};

void	lt_ctl_buttons(t_surface *s, int32_t x, int32_t y, bool primary)
{
	t_lunabtn	b;
	int32_t		i;

	i = 0;
	while (i < 5)
	{
		b.r = lp_rect(x + i * 96, y, 88, 30);
		b.state = g_st[i];
		b.is_default = primary;
		b.focus = (i == 4);
		luna_button(s, &b);
		lt_gal_text(s, b.r, g_nm[i], luna_button_ink(&b));
		i++;
	}
}

void	lt_ctl_choices(t_surface *s, int32_t x, int32_t y)
{
	int32_t	i;

	i = 0;
	while (i < 4)
	{
		luna_checkbox(s, lp_pt(x + i * 64, y), g_st[i], false);
		luna_checkbox(s, lp_pt(x + i * 64 + 22, y), g_st[i], true);
		luna_radio(s, lp_pt(x + i * 64, y + 26), g_st[i], false);
		luna_radio(s, lp_pt(x + i * 64 + 22, y + 26), g_st[i], true);
		i++;
	}
}

void	lt_ctl_fields(t_surface *s, int32_t x, int32_t y)
{
	luna_edit_frame(s, lp_rect(x, y, 130, 28), true);
	luna_edit_frame(s, lp_rect(x + 140, y, 130, 28), false);
	luna_edit_frame(s, lp_rect(x + 280, y - 10, 150, 48), true);
	luna_groupbox(s, lp_rect(x + 440, y - 10, 140, 60), 40);
}

void	lt_ctl_bars(t_surface *s, int32_t x, int32_t y)
{
	int32_t	i;

	i = 0;
	while (i < 4)
	{
		luna_progress(s, lp_rect(x, y + i * 16, 200, 6), i * 33);
		luna_progress(s, lp_rect(x + 220, y + i * 16, 150, 15), i * 33);
		i++;
	}
	luna_scrollbar(s, lp_rect(x + 390, y, 16, 70), LS_NORMAL);
	luna_scrollbar(s, lp_rect(x + 420, y, 16, 70), LS_HOT);
	luna_scrollbar(s, lp_rect(x + 450, y, 16, 70), LS_PRESSED);
	luna_scrollbar(s, lp_rect(x + 480, y, 70, 16), LS_NORMAL);
	luna_scrollbar(s, lp_rect(x + 480, y + 30, 70, 16), LS_DISABLED);
}

void	lt_ctl_menu(t_surface *s, int32_t x, int32_t y)
{
	t_rect	r;
	int32_t	i;

	luna_menu_panel(s, lp_rect(x, y, 170, 108));
	i = 0;
	while (i < 4)
	{
		r = lp_rect(x + 6, y + 6 + i * 24, 158, 22);
		luna_menu_item(s, r, g_st[i]);
		lt_gal_text(s, r, g_nm[i], lp_pal()->ink);
		i++;
	}
}
