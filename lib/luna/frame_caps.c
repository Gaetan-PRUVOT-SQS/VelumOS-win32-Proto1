#include "velum/theme.h"
#include "luna_int.h"

static t_color	cap_color(const t_lcb *cb)
{
	if (cb->st == LS_DISABLED)
		return (lp_pal()->dim);
	if (cb->kind == HT_CLOSE && cb->st != LS_NORMAL)
		return (lp_pal()->danger);
	return (lp_pal()->muted);
}

static void	cap_back(t_surface *s, const t_lcb *cb)
{
	t_lbox	b;
	t_lgrad	g;
	t_lstop	st;

	if (cb->st != LS_HOT && cb->st != LS_PRESSED)
		return ;
	lp_flat(&g, &st, lp_pal()->hover);
	if (cb->st == LS_PRESSED)
		lp_flat(&g, &st, lp_pal()->press);
	b.r = cb->r;
	b.rad = VT_R_CAPBTN;
	b.ring = NULL;
	b.nring = 0;
	b.fill = &g;
	lp_box(s, &b);
}

static void	cap_button(t_surface *s, const t_lunawin *w, const t_lgeo *g,
				int32_t i)
{
	t_lcb	cb;

	cb.r = g->btn[i];
	cb.kind = lg_btn_ht(i);
	cb.active = w->active;
	cb.maximized = w->maximized;
	cb.st = LS_NORMAL;
	if (!(g->flags & (LG_ENABLED << i)))
		cb.st = LS_DISABLED;
	else if (w->pressed == cb.kind)
		cb.st = LS_PRESSED;
	else if (w->hot == cb.kind)
		cb.st = LS_HOT;
	cap_back(s, &cb);
	lp_capglyph(s, &cb, cap_color(&cb));
}

void	lp_capbuttons(t_surface *s, const t_lunawin *w, const t_lgeo *g)
{
	int32_t	i;

	i = -1;
	while (++i < LG_NBTN)
		if (g->flags & (LG_SHOWN << i))
			cap_button(s, w, g, i);
}
