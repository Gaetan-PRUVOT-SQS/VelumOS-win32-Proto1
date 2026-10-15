#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	init_cases(void)
{
	t_surface	s;
	uint32_t	px[12];

	gfx_surface_init(&s, px, 4, 3);
	h_true(s.px == px, "px conserve");
	h_eq_i64("largeur", s.w, 4);
	h_eq_i64("hauteur", s.h, 3);
	h_eq_i64("pas", s.stride, 4);
	h_eq_i64("clip largeur", s.clip.w, 4);
	h_eq_i64("clip hauteur", s.clip.h, 3);
	gfx_surface_init(&s, NULL, 4, 3);
	check_empty("px nul", &s);
	gfx_surface_init(&s, px, 0, 3);
	check_empty("largeur nulle", &s);
	gfx_surface_init(&s, px, 4, -7);
	check_empty("hauteur negative", &s);
	gfx_surface_init(&s, px, INT32_MIN, INT32_MIN);
	check_empty("bornes negatives", &s);
}

static void	check_sub(const t_surface *p, t_rect r, t_rect want)
{
	t_surface	sub;

	sub = gfx_surface_sub(p, r);
	if (rect_empty(want))
	{
		check_empty("sous-surface vide", &sub);
		return ;
	}
	h_true(sub.px == p->px + want.y * p->stride + want.x, "origine px");
	h_eq_i64("sub w", sub.w, want.w);
	h_eq_i64("sub h", sub.h, want.h);
	h_eq_i64("sub pas", sub.stride, p->stride);
	h_eq_i64("sub clip x", sub.clip.x, 0);
	h_eq_i64("sub clip w", sub.clip.w, want.w);
}

static void	sub_cases(void)
{
	t_guard		g;
	t_surface	p;

	guard_new(&g, 80);
	gfx_surface_init(&p, g.px, 10, 8);
	check_sub(&p, rect_make(1, 1, 6, 6), rect_make(1, 1, 6, 6));
	check_sub(&p, rect_make(-3, -3, 6, 6), rect_make(0, 0, 3, 3));
	check_sub(&p, rect_make(8, 6, 9, 9), rect_make(8, 6, 2, 2));
	check_sub(&p, rect_make(20, 20, 3, 3), rect_make(0, 0, 0, 0));
	check_sub(&p, rect_make(0, 0, 0, 5), rect_make(0, 0, 0, 0));
	check_sub(&p, rect_make(0, 0, -4, 5), rect_make(0, 0, 0, 0));
	check_sub(&p, rect_make(-5, -5, INT32_MAX, INT32_MAX),
		rect_make(0, 0, 10, 8));
	check_sub(&p, rect_make(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX),
		rect_make(0, 0, 0, 0));
	check_sub(&p, rect_make(INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX),
		rect_make(0, 0, 0, 0));
	h_true(guard_ok(&g), "garde intacte");
	guard_free(&g);
}

static void	clip_cases(void)
{
	t_guard		g;
	t_surface	p;
	t_surface	sub;

	guard_new(&g, 80);
	gfx_surface_init(&p, g.px, 10, 8);
	gfx_set_clip(&p, rect_make(2, 2, 5, 5));
	sub = gfx_surface_sub(&p, rect_make(1, 1, 6, 6));
	h_eq_i64("clip recalcule x", sub.clip.x, 1);
	h_eq_i64("clip recalcule y", sub.clip.y, 1);
	h_eq_i64("clip recalcule w", sub.clip.w, 5);
	h_eq_i64("clip recalcule h", sub.clip.h, 5);
	sub = gfx_surface_sub(&p, rect_make(0, 0, 2, 2));
	h_true(rect_empty(sub.clip), "clip parent exclut la sous-surface");
	gfx_set_clip(&p, rect_make(-5, -5, 100, 100));
	h_eq_i64("clip borne w", p.clip.w, 10);
	h_eq_i64("clip borne x", p.clip.x, 0);
	gfx_set_clip(&p, rect_make(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX));
	h_eq_i64("clip extreme", p.clip.w, 0);
	gfx_set_clip(&p, rect_make(3, 3, -1, 4));
	h_true(rect_empty(p.clip), "clip negatif vide");
	guard_free(&g);
}

int	main(void)
{
	h_begin("a15/surface");
	h_run("init: partitions valides et invalides", init_cases);
	h_run("sous-surface: partitions et bornes", sub_cases);
	h_run("clip: intersection et recalcul", clip_cases);
	return (h_end());
}
