#include "ref.h"

static void	fuzz_draw(t_fx *fx, int op)
{
	t_surface	*s;
	t_color		c;

	s = &fx->view;
	c = rng_color();
	if (op == 0)
		gfx_put(s, rng_point(s->w), c);
	else if (op == 1)
		(void)gfx_get(s, rng_point(s->w));
	else if (op == 2)
		gfx_fill(s, rng_rect(s->w), c);
	else if (op == 3)
		gfx_hline(s, rng_point(s->w), rng_size(s->w), c);
	else if (op == 4)
		gfx_vline(s, rng_point(s->w), rng_size(s->w), c);
	else if (op == 5)
		gfx_line(s, rng_point(s->w), rng_point(s->w), c);
	else
		gfx_frame(s, rng_rect(s->w), c);
}

static void	fuzz_shape(t_fx *fx, int op)
{
	t_gradient	g;
	t_rrect		rr;
	t_surface	sub;

	g = (t_gradient){rng_rect(fx->view.w), rng_color(), rng_color(),
		(rng_next() & 1) != 0};
	rr = (t_rrect){rng_rect(fx->view.w), rng_color(), rng_size(9),
		rng_coord(9), rng_size(9), rng_coord(9)};
	if (op == 7)
		gfx_gradient(&fx->view, &g);
	else if (op == 8)
		gfx_rrect_fill(&fx->view, &rr);
	else
	{
		sub = gfx_surface_sub(&fx->view, rng_rect(fx->view.w));
		gfx_set_clip(&sub, rng_rect(sub.w));
		gfx_fill(&sub, rng_rect(sub.w), rng_color());
		fx->cv = rect_make(0, 0, fx->view.w, fx->view.h);
	}
}

static void	fuzz_blit(t_fx *dst, t_fx *src, int op)
{
	t_blit	b;

	b = (t_blit){&dst->view, &src->view, rng_rect(dst->view.w),
		rng_rect(src->view.w)};
	if (op == 10)
		gfx_blit(&b);
	else if (op == 11)
		gfx_blit_alpha(&b);
	else if (op == 12)
		gfx_blit_scaled(&b);
	else if (op == 13)
		gfx_blit_smooth(&b);
	else
		gfx_scroll(&dst->view, b.dr, rng_point(dst->view.w));
}

void	fuzz_op(t_fx *dst, t_fx *src, int op)
{
	if (op < 7)
		fuzz_draw(dst, op);
	else if (op < 10)
		fuzz_shape(dst, op);
	else
		fuzz_blit(dst, src, op);
}
