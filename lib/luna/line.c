#include "luna_int.h"

static bool	line_quad(const t_lline *l, t_lpoly *q)
{
	int64_t	dx;
	int64_t	dy;
	int64_t	len;
	int32_t	nx;
	int32_t	ny;

	dx = l->b.x - l->a.x;
	dy = l->b.y - l->a.y;
	len = lp_isqrt((uint64_t)(dx * dx + dy * dy));
	if (len == 0)
		return (false);
	nx = (int32_t)(-dy * l->w / (2 * len));
	ny = (int32_t)(dx * l->w / (2 * len));
	q->n = 4;
	q->p[0] = lp_pt(l->a.x + nx, l->a.y + ny);
	q->p[1] = lp_pt(l->b.x + nx, l->b.y + ny);
	q->p[2] = lp_pt(l->b.x - nx, l->b.y - ny);
	q->p[3] = lp_pt(l->a.x - nx, l->a.y - ny);
	return (true);
}

void	lp_line(t_surface *s, const t_lline *l, t_color c)
{
	t_lpoly	q;
	t_lgrad	g;
	t_lstop	st;
	t_ldisc	d;

	if (s == NULL || l == NULL || l->w <= 0)
		return ;
	lp_flat(&g, &st, c);
	if (line_quad(l, &q))
		lp_poly(s, &q, &g);
	if (!l->round)
		return ;
	d.r = l->w / 2;
	d.r_in = 0;
	d.cx = l->a.x;
	d.cy = l->a.y;
	lp_disc(s, &d, &g);
	d.cx = l->b.x;
	d.cy = l->b.y;
	lp_disc(s, &d, &g);
}
