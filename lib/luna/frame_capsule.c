#include "velum/theme.h"
#include "luna_int.h"

static t_rect	cap_span(const t_lgeo *g)
{
	int32_t	i;
	t_rect	u;

	u = lp_rect(0, 0, 0, 0);
	i = -1;
	while (++i < LG_NBTN)
	{
		if (!(g->flags & (LG_SHOWN << i)))
			continue ;
		if (u.w == 0)
			u = g->btn[i];
		else
			u = lp_rect(u.x, u.y, g->btn[i].x + g->btn[i].w - u.x, u.h);
	}
	return (u);
}

static void	capsule_back(t_surface *s, t_rect u)
{
	t_lbox	b;
	t_lgrad	gr;
	t_lstop	st;

	lp_flat(&gr, &st, lp_pal()->control);
	b.r = lp_rect(u.x - VT_CAPGROUP_PAD, u.y - VT_CAPGROUP_PAD,
			u.w + 2 * VT_CAPGROUP_PAD, u.h + 2 * VT_CAPGROUP_PAD);
	b.rad = VT_R_ITEM;
	b.ring = NULL;
	b.nring = 0;
	b.fill = &gr;
	lp_box(s, &b);
	lp_fill(s, lp_rect(b.r.x + b.rad, b.r.y, b.r.w - 2 * b.rad, 1),
		lp_pal()->rim_top);
}

void	lp_capsule(t_surface *s, const t_lunawin *w, const t_lgeo *g)
{
	t_rect	u;

	u = cap_span(g);
	if (u.w > 0 && !(g->flags & LG_TOOL))
		capsule_back(s, u);
	lp_capbuttons(s, w, g);
}
