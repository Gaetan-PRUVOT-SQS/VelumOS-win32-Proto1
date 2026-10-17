#include "velum/luna.h"
#include "velum/theme.h"
#include "../common/uitext.h"
#include "logon_paint.h"

static void	draw_row_avatars(t_logon *lg)
{
	t_rect		t;
	t_point		p;
	uint32_t	i;

	i = 0;
	while (i < lg->lay.count)
	{
		t = lg->lay.tile[i];
		p.x = lg->lay.card.x + t.x + LOGON_RING + 4;
		p.y = lg->lay.card.y + t.y + LOGON_RING
			+ (LOGON_TILE_H - LOGON_AVATAR_ROW) / 2;
		luna_icon(&lg->ui.win.surface, p, ICON_USER, LOGON_AVATAR_ROW);
		i++;
	}
}

static void	draw_main_avatar(t_logon *lg)
{
	t_rect		a;
	t_rect		n;
	t_uitext	st;
	const char	*name;

	a = lg->lay.avatar;
	n = lg->lay.name;
	luna_icon(&lg->ui.win.surface, (t_point){lg->lay.card.x + a.x
		+ (LOGON_AVATAR - LOGON_AVATAR_ICON) / 2, lg->lay.card.y + a.y
		+ (LOGON_AVATAR - LOGON_AVATAR_ICON) / 2}, ICON_USER,
		LOGON_AVATAR_ICON);
	name = lg->set.list[lg->shown[0]].name;
	st.font = FONT_INS_SEMI_22;
	st.max_w = n.w;
	st.color = VT_INK;
	if (luna_theme_get() == LUNA_THEME_DARK)
		st.color = VT_INK_D;
	st.at.x = lg->lay.card.x + n.x + (n.w - ui_text_width(st.font, name)) / 2;
	st.at.y = lg->lay.card.y + n.y + (n.h - ui_font_height(st.font)) / 2;
	ui_text(&lg->ui.win.surface, &st, name);
}

void	logon_draw_avatars(t_logon *lg)
{
	if (lg->lay.count > 1)
		draw_row_avatars(lg);
	else if (lg->lay.count == 1)
		draw_main_avatar(lg);
}
