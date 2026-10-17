#include "velum/luna.h"
#include "velum/theme.h"
#include "../common/layout.h"
#include "../common/uitext.h"
#include "logon_glass.h"
#include "logon_paint.h"

static t_color	chip_ink(void)
{
	if (luna_theme_get() == LUNA_THEME_DARK)
		return (VT_INK_D);
	return (VT_INK);
}

static int32_t	chip_width(const char *label, int icon)
{
	int32_t	w;

	w = LOGON_CHIP_TAIL + ui_text_width(FONT_INS_SEMI_14, label);
	if (icon)
		w += LOGON_CHIP_PAD + LOGON_CHIP_ICON + LOGON_CHIP_GAP;
	else
		w += LOGON_CHIP_TAIL;
	return (w);
}

static void	draw_chip(t_logon *lg, t_rect chip, const char *label, int icon)
{
	t_surface	*s;
	t_uitext	st;
	t_point		p;

	s = &lg->ui.win.surface;
	logon_glass_open(chip);
	luna_glass(s, chip, LUNA_GLASS_CHIP);
	st.at.x = chip.x + LOGON_CHIP_TAIL;
	if (icon)
	{
		p.x = chip.x + LOGON_CHIP_PAD;
		p.y = chip.y + (chip.h - LOGON_CHIP_ICON) / 2;
		luna_icon(s, p, ICON_USER, LOGON_CHIP_ICON);
		st.at.x = p.x + LOGON_CHIP_ICON + LOGON_CHIP_GAP;
	}
	st.font = FONT_INS_SEMI_14;
	st.color = chip_ink();
	st.max_w = chip.x + chip.w - st.at.x;
	st.at.y = chip.y + (chip.h - ui_font_height(st.font)) / 2;
	ui_text(s, &st, label);
}

void	logon_draw_chips(t_logon *lg)
{
	t_rect	chip;

	if (lg->lay.chips.w <= 0)
		return ;
	chip = lay_rect(lg->lay.chips.x, lg->lay.chips.y,
			chip_width(LOGON_CHIP_USER, 1), lg->lay.chips.h);
	draw_chip(lg, chip, LOGON_CHIP_USER, 1);
	chip.x += chip.w + LOGON_CHIP_SPACE;
	chip.w = chip_width(LOGON_CHIP_LANG, 0);
	draw_chip(lg, chip, LOGON_CHIP_LANG, 0);
}
