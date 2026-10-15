#include "luna_int.h"

void	lp_op_disc(t_surface *s, const t_lscale *sc, const t_lop *op)
{
	t_lg2	o;
	t_ldisc	d;

	d.cx = lp_op_x(sc, op->a[0]);
	d.cy = lp_op_y(sc, op->a[1]);
	d.r = lp_op_len(sc, op->a[2]);
	d.r_in = 0;
	lp_op_grad(op, 1, lp_floor16(2 * d.r) + 2, &o);
	lp_disc(s, &d, &o.g);
}

void	lp_op_ring(t_surface *s, const t_lscale *sc, const t_lop *op)
{
	t_lgrad	g;
	t_lstop	st;
	t_ldisc	d;

	d.cx = lp_op_x(sc, op->a[0]);
	d.cy = lp_op_y(sc, op->a[1]);
	d.r = lp_op_len(sc, op->a[2]);
	d.r_in = lp_op_len(sc, op->a[3]);
	lp_flat(&g, &st, op->c0);
	lp_disc(s, &d, &g);
}

void	lp_op_poly(t_surface *s, const t_lscale *sc, const t_lop *op)
{
	t_lg2	o;
	t_lpoly	q;
	int32_t	i;

	q.n = op->n;
	i = 0;
	while (i < q.n && i < LP_PMAX)
	{
		q.p[i].x = lp_op_x(sc, op->a[2 * i]);
		q.p[i].y = lp_op_y(sc, op->a[2 * i + 1]);
		i++;
	}
	if (q.n < 3 || q.n > LP_PMAX)
		return ;
	lp_op_grad(op, 1, lp_poly_box(&q).h - 1, &o);
	lp_poly(s, &q, &o.g);
}

void	lp_op_line(t_surface *s, const t_lscale *sc, const t_lop *op)
{
	t_lline	l;

	l.a = lp_pt(lp_op_x(sc, op->a[0]), lp_op_y(sc, op->a[1]));
	l.b = lp_pt(lp_op_x(sc, op->a[2]), lp_op_y(sc, op->a[3]));
	l.w = lp_op_len(sc, op->a[4]) / 2;
	l.round = op->n != 0;
	lp_line(s, &l, op->c0);
}
