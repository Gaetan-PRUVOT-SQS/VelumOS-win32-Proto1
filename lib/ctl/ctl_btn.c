#include "ctl_int.h"

t_rect	ctl_btn_face(const t_ctl *c)
{
	t_rect	r;

	r = c->rect;
	if (r.w <= 2 * LUNA_FOCUS_MARGIN || r.h <= 2 * LUNA_FOCUS_MARGIN)
		return (r);
	return (rect_make(r.x + LUNA_FOCUS_MARGIN, r.y + LUNA_FOCUS_MARGIN,
			r.w - 2 * LUNA_FOCUS_MARGIN, r.h - 2 * LUNA_FOCUS_MARGIN));
}

void	ctl_btn_paint(t_ctlroot *r, t_ctl *c)
{
	t_lunabtn	b;
	t_ctltext	tx;

	b.r = ctl_btn_face(c);
	b.state = ctl_lstate(c);
	b.is_default = (c->flags & CTL_DEFAULT) != 0;
	b.focus = (r->focus == c);
	luna_button(r->surface, &b);
	ctl_text_init(&tx, c);
	tx.box = rect_make(b.r.x + CTL_PAD, b.r.y, b.r.w - 2 * CTL_PAD, b.r.h);
	if (tx.box.w < 0)
		tx.box.w = 0;
	tx.enabled = true;
	tx.color = luna_button_ink(&b);
	tx.flags |= CTL_ALIGN_CENTER;
	if (b.is_default && b.state != LS_DISABLED)
		tx.flags |= CTL_BOLD;
	if (b.state == LS_PRESSED)
	{
		tx.box.x++;
		tx.box.y++;
	}
	ctl_text_draw(r->surface, &tx);
}

bool	ctl_btn_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev)
{
	if (ev->type != INP_KEY_DOWN)
		return (false);
	if (ev->code != VK_SPACE && ev->code != VK_RETURN)
		return (false);
	if (ev->mods & (INPM_ALT | INPM_CTRL))
		return (false);
	if (!(ev->mods & INPM_REPEAT))
		ctl_activate(r, c);
	return (true);
}

bool	ctl_btn_activate(t_ctlroot *r, t_ctl *c)
{
	ctl_notify(r, c, CN_CLICKED);
	return (true);
}
