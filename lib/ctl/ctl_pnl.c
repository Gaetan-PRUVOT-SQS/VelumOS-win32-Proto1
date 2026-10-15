#include "ctl_int.h"

void	ctl_pnl_paint(t_ctlroot *r, t_ctl *c)
{
	if (c->flags & CTL_NOBACK)
		return ;
	gfx_fill(r->surface, c->rect, luna_color_window());
}

void	ctl_lbl_paint(t_ctlroot *r, t_ctl *c)
{
	t_ctltext	tx;

	ctl_text_init(&tx, c);
	ctl_text_draw(r->surface, &tx);
}

bool	ctl_lbl_activate(t_ctlroot *r, t_ctl *c)
{
	return (ctl_focus_after(r, c));
}

void	ctl_grp_paint(t_ctlroot *r, t_ctl *c)
{
	t_ctltext	tx;
	int32_t		w;

	if (!ctl_font_of(c))
		return ;
	ctl_text_init(&tx, c);
	w = ctl_text_width(&tx);
	if (w > 0)
		w += 2 * CTL_GAP;
	luna_groupbox(r->surface, c->rect, w);
	tx.box.x += 2 * CTL_GAP;
	tx.box.h = ctl_font_of(c)->height;
	ctl_text_draw(r->surface, &tx);
}
