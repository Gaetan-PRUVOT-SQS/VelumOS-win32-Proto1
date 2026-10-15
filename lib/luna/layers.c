#include "luna_int.h"

static void	layer_shape(const t_llayers *l, int32_t i, t_lrr *rr)
{
	int32_t	k;

	rr->r = lp_inset(l->r, i);
	k = 0;
	while (k < 4)
	{
		rr->rad[k] = lp_max(l->rad[k] - i, 0);
		k++;
	}
	rr->hole = l->hole;
}

void	lp_layers(t_surface *s, const t_llayers *l)
{
	t_lrr	rr;
	t_lstop	one;
	t_lgrad	flat;
	int32_t	i;

	if (s == NULL || l == NULL || l->fill == NULL)
		return ;
	i = 0;
	while (i <= l->nring)
	{
		layer_shape(l, i, &rr);
		if (rr.r.w <= 0 || rr.r.h <= 0)
			return ;
		rr.g = l->fill;
		if (i < l->nring)
		{
			lp_flat(&flat, &one, l->ring[i]);
			rr.g = &flat;
		}
		rr.y0 = rr.r.y - l->skip * (i == l->nring);
		lp_rr_paint(s, &rr);
		i++;
	}
}

void	lp_box(t_surface *s, const t_lbox *b)
{
	t_llayers	l;

	if (b == NULL)
		return ;
	l.r = b->r;
	l.rad[0] = b->rad;
	l.rad[1] = b->rad;
	l.rad[2] = b->rad;
	l.rad[3] = b->rad;
	l.ring = b->ring;
	l.nring = b->nring;
	l.fill = b->fill;
	l.hole = lp_rect(0, 0, 0, 0);
	l.skip = 0;
	lp_layers(s, &l);
}
