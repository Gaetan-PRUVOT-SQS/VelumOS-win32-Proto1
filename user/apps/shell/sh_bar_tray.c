#include "velum/luna_dock.h"
#include "velum/theme.h"
#include "../common/uitext.h"
#include "shell.h"

t_color	sh_ink(void)
{
	if (luna_theme_get() == LUNA_THEME_DARK)
		return (VT_INK_D);
	return (VT_INK);
}

static t_color	muted_ink(void)
{
	if (luna_theme_get() == LUNA_THEME_DARK)
		return (VT_MUTED_D);
	return (VT_MUTED);
}

static void	put_text(t_shell *sh, t_uitext *st, const char *text, t_rect box)
{
	st->at.x = box.x + (box.w - ui_text_width(st->font, text)) / 2;
	st->max_w = box.w;
	ui_text(&sh->bar.surface, st, text);
}

static void	put_clock(t_shell *sh, t_rect box)
{
	t_uitext	st;
	int32_t		h_time;
	int32_t		h_date;
	int32_t		top;

	h_time = ui_font_height(FONT_INS_BOLD_13);
	h_date = ui_font_height(FONT_INS_11);
	top = box.y + (box.h - h_time - h_date) / 2;
	st.font = FONT_INS_BOLD_13;
	st.at.y = top;
	st.color = sh_ink();
	put_text(sh, &st, sh->clock, box);
	st.font = FONT_INS_11;
	st.at.y = top + h_time;
	st.color = muted_ink();
	put_text(sh, &st, sh->date, box);
}

void	sh_bar_tray(t_shell *sh)
{
	t_traygeo	g;
	t_uitext	st;

	luna_tray(&sh->bar.surface, sh->reg.tray);
	luna_tray_geo(sh->reg.tray, &g);
	st.font = FONT_INS_SEMI_12;
	st.at.y = g.lang.y + (g.lang.h - ui_font_height(st.font)) / 2;
	st.color = muted_ink();
	put_text(sh, &st, "FR", g.lang);
	put_clock(sh, g.clock);
}
