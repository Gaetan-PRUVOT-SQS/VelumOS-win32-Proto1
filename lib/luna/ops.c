#include "luna_int.h"

int32_t	lp_op_x(const t_lscale *sc, int32_t v)
{
	return (sc->ox * 16 + (v * sc->num * 16) / sc->den);
}

int32_t	lp_op_y(const t_lscale *sc, int32_t v)
{
	return (sc->oy * 16 + (v * sc->num * 16) / sc->den);
}

int32_t	lp_op_len(const t_lscale *sc, int32_t v)
{
	return ((v * sc->num * 16) / sc->den);
}

void	lp_op_grad(const t_lop *op, int32_t at0, int32_t at1, t_lg2 *o)
{
	o->st[0].at = at0;
	o->st[0].c = op->c0;
	o->st[1].at = at1;
	o->st[1].c = op->c1;
	o->g.st = o->st;
	o->g.n = 2;
	o->g.tint = 0;
	o->g.tint_t = 0;
}

void	lp_ops(t_surface *s, const t_lscale *sc, const t_lop *ops, int32_t n)
{
	int32_t	i;

	if (s == NULL || sc == NULL || ops == NULL || sc->den <= 0)
		return ;
	i = 0;
	while (i < n)
	{
		if (ops[i].kind == LOP_RECT)
			lp_op_rect(s, sc, &ops[i]);
		else if (ops[i].kind == LOP_RRECT)
			lp_op_rrect(s, sc, &ops[i]);
		else if (ops[i].kind == LOP_DISC)
			lp_op_disc(s, sc, &ops[i]);
		else if (ops[i].kind == LOP_POLY)
			lp_op_poly(s, sc, &ops[i]);
		else if (ops[i].kind == LOP_LINE)
			lp_op_line(s, sc, &ops[i]);
		else if (ops[i].kind == LOP_RING)
			lp_op_ring(s, sc, &ops[i]);
		i++;
	}
}
