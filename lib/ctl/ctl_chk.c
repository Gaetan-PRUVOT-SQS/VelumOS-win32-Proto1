#include "ctl_int.h"

static void	chk_ring(t_ctlroot *r, const t_ctl *c, const t_ctltext *tx)
{
	t_rect	ring;
	int32_t	m;

	if (r->focus != c || !ctl_font_of(c))
		return ;
	m = LUNA_FOCUS_MARGIN;
	ring.x = tx->box.x - m;
	ring.w = ctl_text_width(tx) + 2 * m;
	ring.h = ctl_font_of(c)->height + 2 * m;
	ring.y = c->rect.y + (c->rect.h - ring.h) / 2;
	ctl_focus_ring(r->surface, rect_intersect(ring, c->rect));
}

void	ctl_chk_paint(t_ctlroot *r, t_ctl *c)
{
	t_point		p;
	t_ctltext	tx;
	bool		on;
	int32_t		box;

	on = (c->flags & CTL_CHECKED) != 0;
	box = CTL_BOX;
	if (c->type == CT_RADIO)
		box = CTL_RADIO_BOX;
	p.x = c->rect.x;
	p.y = c->rect.y + (c->rect.h - box) / 2;
	if (c->type == CT_RADIO)
		ctl_radio_draw(r->surface, p, ctl_lstate(c), on);
	else
		luna_checkbox(r->surface, p, ctl_lstate(c), on);
	ctl_text_init(&tx, c);
	tx.box.x += box + CTL_GAP;
	tx.box.w -= box + CTL_GAP;
	if (tx.box.w < 0)
		tx.box.w = 0;
	ctl_text_draw(r->surface, &tx);
	chk_ring(r, c, &tx);
}

bool	ctl_chk_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev)
{
	if (ev->type != INP_KEY_DOWN || ev->code != VK_SPACE)
		return (false);
	if (ev->mods & (INPM_ALT | INPM_CTRL))
		return (false);
	if (!(ev->mods & INPM_REPEAT))
		ctl_activate(r, c);
	return (true);
}

bool	ctl_chk_activate(t_ctlroot *r, t_ctl *c)
{
	c->flags ^= CTL_CHECKED;
	ctl_dirty_add(r, c->rect);
	ctl_notify(r, c, CN_CLICKED);
	return (true);
}
