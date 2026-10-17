#include "luna_int.h"

uint32_t	lg_btn_ht(int32_t i)
{
	if (i == 0)
		return (HT_MINBUTTON);
	if (i == 1)
		return (HT_MAXBUTTON);
	return (HT_CLOSE);
}

bool	lg_inside(t_rect a, t_rect b)
{
	if (a.w <= 0 || a.h <= 0)
		return (false);
	if (a.x < b.x || a.y < b.y)
		return (false);
	if ((int64_t)a.x + a.w > (int64_t)b.x + b.w)
		return (false);
	return ((int64_t)a.y + a.h <= (int64_t)b.y + b.h);
}

static void	lg_btn_params(const t_lgeo *g, t_lbp *bp)
{
	const t_lunadetail	*d;

	d = lm_detail();
	bp->size = lm_metrics()->btn_w;
	bp->height = lm_metrics()->btn_h;
	bp->top = d->btn_top;
	bp->right = d->btn_right;
	bp->left = g->caption.x;
	if ((g->flags & LG_SYSMENU) && !(g->flags & LG_TOOL))
		bp->left += d->sysmenu_w;
	if (g->flags & LG_MAX)
	{
		bp->top = d->btn_top_max;
		bp->right = d->btn_right_max;
	}
	if (g->flags & LG_TOOL)
	{
		bp->size = d->tool_btn;
		bp->height = d->tool_btn;
		bp->top = d->tool_btn_top;
		bp->right = d->tool_btn_right;
	}
}

static uint32_t	lg_btn_flags(const t_lunawin *w, const t_lgeo *g, int32_t i)
{
	uint32_t	f;
	uint32_t	boxes;

	boxes = w->style & (WS_MINBOX | WS_MAXBOX);
	if (!(w->style & WS_SYSMENU))
		return (0);
	if (i == 2)
		return ((LG_SHOWN | LG_ENABLED) << i);
	if ((g->flags & LG_TOOL) || boxes == 0)
		return (0);
	f = LG_SHOWN << i;
	if (i == 0 && (w->style & WS_MINBOX))
		f |= LG_ENABLED << i;
	if (i == 1 && (w->style & WS_MAXBOX))
		f |= LG_ENABLED << i;
	return (f);
}

void	lg_buttons(const t_lunawin *w, t_lgeo *g)
{
	t_lbp		bp;
	t_rect		r;
	uint32_t	f;
	int32_t		i;
	int32_t		slot;

	lg_btn_params(g, &bp);
	i = 0;
	while (i < LG_NBTN)
	{
		slot = LG_NBTN - 1 - i;
		r = lp_rect(g->outer.x + g->outer.w - bp.right - (slot + 1) * bp.size
				- slot * lm_detail()->btn_gap, g->outer.y + bp.top, bp.size,
				bp.height);
		g->btn[i] = lp_rect(g->outer.x, g->outer.y, 0, 0);
		f = lg_btn_flags(w, g, i);
		if (f != 0 && lg_inside(r, g->caption) && r.x >= bp.left)
		{
			g->btn[i] = r;
			g->flags |= f;
		}
		i++;
	}
}
