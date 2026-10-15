#include "ctl_int.h"

static bool	same_rect(t_rect a, t_rect b)
{
	return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
}

void	ctl_set_text(t_ctlroot *r, t_ctl *c, const char *text)
{
	char	tmp[CTL_TEXT_MAX];

	if (!r || !r->root || !c || !ctl_in_tree(r, c))
		return ;
	ctl_copy_text(tmp, text);
	if (strcmp(tmp, c->text) == 0)
		return ;
	memcpy(c->text, tmp, strlen(tmp) + 1);
	if (c->type == CT_EDIT)
		ctl_edt_clamp(c, c->priv);
	ctl_dirty_add(r, c->rect);
}

void	ctl_set_flag(t_ctlroot *r, t_ctl *c, uint32_t flag, bool on)
{
	uint32_t	old;

	if (!r || !r->root || !c || !ctl_in_tree(r, c))
		return ;
	old = c->flags;
	flag &= CTL_FLAGS_ALL;
	if (on)
		c->flags |= flag;
	else
		c->flags &= ~flag;
	if (c->type == CT_RADIO && on && (flag & CTL_CHECKED))
		ctl_radio_select(r, c);
	if ((old ^ c->flags) & CTL_FLAGS_VISUAL)
		ctl_dirty_add(r, c->rect);
	ctl_refs_check(r);
}

int	ctl_set_rect(t_ctlroot *r, t_ctl *c, t_rect rect)
{
	if (!r || !r->root || !c || !ctl_in_tree(r, c))
		return (E_INVAL);
	if (!ctl_rect_ok(rect))
		return (E_INVAL);
	if (same_rect(rect, c->rect))
		return (0);
	ctl_dirty_add(r, c->rect);
	c->rect = rect;
	ctl_dirty_add(r, c->rect);
	if (c->type == CT_EDIT)
		ctl_edt_scroll(c, c->priv);
	return (0);
}

void	ctl_set_font(t_ctlroot *r, t_ctl *c, t_fontid font)
{
	uint32_t	next;

	if (!r || !r->root || !c || !ctl_in_tree(r, c))
		return ;
	if ((uint32_t)font > CTL_FONT_AUTO)
		return ;
	next = 0;
	if ((uint32_t)font < FONT_IDS)
		next = (uint32_t)font + 1;
	if (next == c->font)
		return ;
	c->font = next;
	if (c->type == CT_EDIT)
		ctl_edt_scroll(c, c->priv);
	ctl_dirty_add(r, c->rect);
}
