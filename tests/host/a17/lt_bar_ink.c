#include <stdio.h>
#include "harness.h"
#include "lt_bar_ink.h"

static uint32_t			g_scr[LKB_SCR];

static const char		*g_theme[2] = {"clair", "sombre"};

t_color	lkb_ink(bool dark)
{
	if (dark)
		return (VT_INK_D);
	return (VT_INK);
}

void	lkb_begin(t_lkbar *b, t_lt *t, bool dark)
{
	t_rect	bar;

	b->t = t;
	b->dark = dark;
	luna_theme_set(LUNA_THEME_LIGHT);
	if (dark)
		luna_theme_set(LUNA_THEME_DARK);
	luna_transparency_set(true);
	luna_glass_scratch(g_scr, LKB_SCR);
	bar = lp_rect(0, LKB_H - LKB_BAR_H, LKB_W, LKB_BAR_H);
	luna_wallpaper(&t->s, lp_rect(0, 0, LKB_W, LKB_H));
	luna_taskbar(&t->s, bar);
	luna_taskbar_layout(bar, &b->lay);
}

void	lkb_task(const t_lkbar *b, int32_t i, t_lunastate st)
{
	t_rect	r;
	t_rect	label;
	char	name[64];

	r = lp_rect(b->lay.tasks.x + i * (LKB_TASK_W + VT_DOCK_GAP),
			b->lay.tasks.y, LKB_TASK_W, b->lay.tasks.h);
	luna_task_button(&b->t->s, r, st);
	label = lp_rect(r.x + VT_TASK_PAD_L, r.y,
			r.w - VT_TASK_PAD_L - VT_TASK_PAD_R, r.h);
	snprintf(name, sizeof(name), "libelle de tache, etat %d, theme %s",
		(int)st, g_theme[b->dark]);
	lk_check(name, lk_worst(&b->t->s, lk_band(label, LK_UI),
			lkb_ink(b->dark)));
}

void	lkb_clock(const t_lkbar *b)
{
	t_traygeo	g;
	char		name[64];

	luna_tray(&b->t->s, b->lay.tray);
	luna_tray_geo(b->lay.tray, &g);
	snprintf(name, sizeof(name), "horloge sur la zone de notification, %s",
		g_theme[b->dark]);
	lk_check(name, lk_worst(&b->t->s, lk_band(g.clock, LK_UI),
			lkb_ink(b->dark)));
	h_true(lk_worst(&b->t->s, lk_band(g.clock, LK_UI), lkb_ink(!b->dark))
		< LK_AA, "controle de sensibilite, encre du theme oppose refusee");
}
