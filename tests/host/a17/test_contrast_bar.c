#include "harness.h"
#include "lt_bar_ink.h"

static t_lt		g_t;
static t_color	g_emblem;

static void	bar_start(const t_lkbar *b)
{
	t_rect	r;
	t_point	mid;
	int32_t	st;

	r = b->lay.start;
	mid = lp_pt(r.x + r.w / 2, r.y + r.h / 2);
	st = LS_NORMAL;
	while (st <= LS_PRESSED)
	{
		luna_start_button(&b->t->s, r, (t_lunastate)st);
		if (g_emblem == 0)
			g_emblem = gfx_get(&b->t->s, mid);
		h_eq_u64("embleme opaque, identique selon l'etat et le theme",
			gfx_get(&b->t->s, mid), g_emblem);
		st++;
	}
}

static void	bar_theme(bool dark)
{
	t_lkbar	b;
	int32_t	st;

	lkb_begin(&b, &g_t, dark);
	st = LS_NORMAL;
	while (st <= LS_PRESSED)
	{
		lkb_task(&b, st, (t_lunastate)st);
		st++;
	}
	lkb_clock(&b);
	bar_start(&b);
}

static void	bar_light(void)
{
	bar_theme(false);
}

static void	bar_dark(void)
{
	bar_theme(true);
}

int	main(void)
{
	if (lt_open(&g_t, LKB_W, LKB_H) != 0)
		return (1);
	h_begin("a17/contrast_bar");
	h_run("P25 encre du theme sur le verre du dock, clair", bar_light);
	h_run("P25 encre du theme sur le verre du dock, sombre", bar_dark);
	luna_glass_scratch(NULL, 0);
	luna_theme_set(LUNA_THEME_LIGHT);
	lt_close(&g_t);
	return (h_end());
}
