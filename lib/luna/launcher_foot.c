#include "launcher_int.h"

static void	foot_button(t_surface *s, t_rect r, int32_t st)
{
	t_cpal	p;
	t_color	c;

	p = lp_pal();
	c = p->control;
	if (st != 0)
		c = p->sel;
	lp_launch_fill(s, r, r.h / 2, c);
	lp_fill(s, lp_rect(r.x + r.h / 2, r.y, r.w - r.h, 1),
		lp_launch_tone(LL_BTN_RIM_L, LL_BTN_RIM_D));
	if (st & LL_ST_FOCUS)
		lp_launch_focus(s, r, r.h / 2);
}

static void	foot_user(t_surface *s, const t_launcher *l,
				const t_launcherlayout *lay)
{
	t_ltext	t;

	luna_icon(s, lp_pt(lay->avatar.x, lay->avatar.y), ICON_USER, LL_AVATAR);
	if (l->user == NULL)
		return ;
	t.font = FONT_UI_BOLD;
	t.color = lp_pal()->ink;
	t.shadow = 0;
	t.text = l->user;
	t.box = lay->user;
	t.align = LA_LEFT;
	lp_text(s, &t);
}

static void	foot_logoff(t_surface *s, const t_launcher *l,
				const t_launcherlayout *lay)
{
	t_ltext	t;

	foot_button(s, lay->logoff, lp_launch_state(l, LUNA_LAUNCHER_LOGOFF));
	t.font = FONT_UI_BOLD;
	t.color = lp_pal()->ink;
	t.shadow = 0;
	t.text = "Fermer la session";
	t.box = lay->logoff;
	t.align = LA_CENTER;
	lp_text(s, &t);
}

static void	foot_power(t_surface *s, const t_launcher *l,
				const t_launcherlayout *lay)
{
	t_rect	r;

	r = lay->poweroff;
	foot_button(s, r, lp_launch_state(l, LUNA_LAUNCHER_POWER));
	luna_icon(s, lp_pt(r.x + (r.w - LL_POWER_ICON) / 2,
			r.y + (r.h - LL_POWER_ICON) / 2), ICON_POWER, LL_POWER_ICON);
}

void	lp_launch_foot(t_surface *s, const t_launcher *l,
			const t_launcherlayout *lay)
{
	lp_fill(s, lay->sep, lp_launch_tone(LL_SEP_L, LL_SEP_D));
	foot_user(s, l, lay);
	foot_logoff(s, l, lay);
	foot_power(s, l, lay);
}
