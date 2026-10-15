#include "gfx_int.h"

static void	line_axes(t_line *c, t_point a, t_point b)
{
	int64_t	dmaj;
	int64_t	dmin;

	c->maj0 = a.x;
	c->min0 = a.y;
	dmaj = (int64_t)b.x - a.x;
	dmin = (int64_t)b.y - a.y;
	if (c->steep)
	{
		c->maj0 = a.y;
		c->min0 = a.x;
		dmaj = (int64_t)b.y - a.y;
		dmin = (int64_t)b.x - a.x;
	}
	c->len = dmaj;
	c->dmin = (uint64_t)gfx_abs64(dmin);
	c->sgn = 1;
	if (dmin < 0)
		c->sgn = -1;
}

static void	line_setup(t_line *c, t_point a, t_point b)
{
	t_point	t;

	c->steep = gfx_abs64((int64_t)b.y - a.y) > gfx_abs64((int64_t)b.x - a.x);
	if ((c->steep && b.y < a.y) || (!c->steep && b.x < a.x))
	{
		t = a;
		a = b;
		b = t;
	}
	line_axes(c, a, b);
}

void	gfx_line(t_surface *s, t_point a, t_point b, t_color c)
{
	t_line	ln;

	if ((c >> 24) == 0)
		return ;
	ln.clip = gfx_clipbox(s);
	if (gfx_box_empty(ln.clip))
		return ;
	line_setup(&ln, a, b);
	gfx_line_walk(s, &ln, c);
}
