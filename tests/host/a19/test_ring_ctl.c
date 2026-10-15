#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	paint_all(t_fakeui *u)
{
	ctl_invalidate(&u->r, u->r.root);
	ctl_paint(&u->r);
}

static int	corners(const t_fakeui *u, t_rect r, t_color col)
{
	int	n;

	n = fake_px_count(&u->s, rect_make(r.x, r.y, 1, 1), col);
	n += fake_px_count(&u->s, rect_make(r.x + r.w - 1, r.y, 1, 1), col);
	n += fake_px_count(&u->s, rect_make(r.x, r.y + r.h - 1, 1, 1), col);
	n += fake_px_count(&u->s,
			rect_make(r.x + r.w - 1, r.y + r.h - 1, 1, 1), col);
	return (n);
}

static void	list_rows(void)
{
	t_fakeui	u;
	t_ctl		*l;
	t_rect		row;
	int			area;

	fake_begin(&u, 160, 120);
	l = fake_list(&u, 3);
	ctl_focus(&u.r, l);
	paint_all(&u);
	h_eq_i64("liste sans choix : aucune ligne selectionnee",
		ctl_list_selected(l), -1);
	row = ctl_lst_item_rect(l, 0);
	area = row.w * row.h - (row.w - 4) * (row.h - 4);
	h_eq_i64("ligne de focus : anneau accent de 2 px",
		fake_px_count(&u.s, row, luna_color_accent()), area);
	ctl_list_select(&u.r, l, 1);
	paint_all(&u);
	row = ctl_lst_item_rect(l, 1);
	h_eq_i64("ligne choisie : pas d'accent sur la selection",
		fake_px_count(&u.s, row, luna_color_accent()), 0);
	h_eq_i64("ligne choisie : anneau lisible aux 4 coins",
		corners(&u, row, ctl_color_selected_text()), 4);
	fake_done(&u);
}

static void	scroll_bar(void)
{
	t_fakeui	u;
	t_ctl		*s;
	t_rect		r;

	fake_begin(&u, 100, 150);
	r = rect_make(10, 10, 17, 100);
	s = fake_add(&u, fake_spec(CT_SCROLL, 5, r, NULL));
	ctl_scroll_set(&u.r, s, 100, 10);
	ctl_focus(&u.r, s);
	paint_all(&u);
	h_eq_i64("ascenseur : anneau accent sur le bord du controle",
		fake_px_count(&u.s, r, luna_color_accent()),
		r.w * r.h - (r.w - 4) * (r.h - 4));
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/ring-ctl");
	h_run("liste : ligne de focus et ligne choisie", list_rows);
	h_run("ascenseur : anneau sur le bord", scroll_bar);
	return (h_end());
}
