#include "fake.h"
#include "ctl_int.h"

static bool	accent_in(const t_fakeui *u, t_rect area)
{
	return (fake_px_count(&u->s, area, luna_color_accent()) > 0);
}

static bool	button_focus(const t_ctl *c)
{
	int			i;
	t_fakecall	*k;
	t_rect		face;

	face = rect_make(c->rect.x + 4, c->rect.y + 4, c->rect.w - 8,
			c->rect.h - 8);
	i = fake_log_count() - 1;
	while (i >= 0)
	{
		k = fake_log_get(i);
		if (k->kind == FK_BUTTON && fake_rect_eq(k->r, face))
			return ((k->b & 2) != 0);
		i--;
	}
	return (false);
}

static bool	edit_caret(const t_fakeui *u, const t_ctl *c)
{
	t_rect	col;

	col = rect_make(c->rect.x + 3, c->rect.y + (c->rect.h - 11) / 2, 1, 11);
	return (fake_px_count(&u->s, col, 0xff000000) == 11);
}

static bool	list_ring(const t_fakeui *u, const t_ctl *c)
{
	int		sel;
	t_rect	corner;
	t_color	want;

	sel = ctl_list_selected(c);
	want = luna_color_accent();
	if (sel < 0)
		sel = 0;
	else
		want = ctl_color_selected_text();
	corner = ctl_lst_item_rect(c, sel);
	corner.w = 1;
	corner.h = 1;
	return (fake_px_count(&u->s, corner, want) == 1);
}

bool	fake_focus_visible(t_fakeui *u, const t_ctl *c)
{
	ctl_invalidate(&u->r, u->r.root);
	fake_log_clear();
	ctl_paint(&u->r);
	if (c->type == CT_BUTTON)
		return (button_focus(c));
	if (c->type == CT_EDIT)
		return (edit_caret(u, c));
	if (c->type == CT_LIST)
		return (list_ring(u, c));
	return (accent_in(u, c->rect));
}
