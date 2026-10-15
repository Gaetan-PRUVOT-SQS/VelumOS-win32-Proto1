#include "luna_int.h"

static t_rect	op_box(const t_lscale *sc, const t_lop *op)
{
	int32_t	x0;
	int32_t	y0;
	int32_t	x1;
	int32_t	y1;

	x0 = lp_round16(lp_op_x(sc, op->a[0]));
	y0 = lp_round16(lp_op_y(sc, op->a[1]));
	x1 = lp_round16(lp_op_x(sc, op->a[2]));
	y1 = lp_round16(lp_op_y(sc, op->a[3]));
	x1 = lp_max(x1, x0 + 1);
	y1 = lp_max(y1, y0 + 1);
	return (lp_rect(x0, y0, x1 - x0, y1 - y0));
}

void	lp_op_rect(t_surface *s, const t_lscale *sc, const t_lop *op)
{
	t_lg2	o;
	t_rect	r;

	r = op_box(sc, op);
	lp_op_grad(op, 0, lp_max(r.h - 1, 1), &o);
	lp_vfill(s, r, &o.g);
}

void	lp_op_rrect(t_surface *s, const t_lscale *sc, const t_lop *op)
{
	t_lg2	o;
	t_lrr	rr;
	int32_t	r;
	int32_t	k;

	rr.r = op_box(sc, op);
	r = lp_round16(lp_op_len(sc, op->a[4]));
	k = 0;
	while (k < 4)
	{
		rr.rad[k] = 0;
		if (op->n == 0 || ((op->n >> k) & 1))
			rr.rad[k] = r;
		k++;
	}
	lp_op_grad(op, 0, lp_max(rr.r.h - 1, 1), &o);
	rr.g = &o.g;
	rr.y0 = rr.r.y;
	rr.hole = lp_rect(0, 0, 0, 0);
	lp_rr_paint(s, &rr);
}

void	lp_ops_flat(t_surface *s, const t_lscale *sc, const t_loplist *l)
{
	t_lop	one;
	int32_t	i;

	i = 0;
	while (l != NULL && i < l->n)
	{
		one = l->ops[i];
		one.c0 = l->flat;
		one.c1 = l->flat;
		lp_ops(s, sc, &one, 1);
		i++;
	}
}
