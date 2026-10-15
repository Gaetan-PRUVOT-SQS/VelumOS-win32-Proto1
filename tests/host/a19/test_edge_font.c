#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static t_ctl	*add_list(t_fakeui *u, int items)
{
	t_ctl	*l;
	int		i;

	l = fake_add(u, fake_spec(CT_LIST, 7, rect_make(10, 10, 100, 60), ""));
	i = 0;
	while (l && i < items)
	{
		ctl_list_add(&u->r, l, "ligne");
		i++;
	}
	return (l);
}

static void	list_rows_follow_font(void)
{
	t_fakeui	u;
	t_ctl		*l;
	int32_t		pad;

	pad = CTL_ROW_PAD;
	fake_begin(&u, 200, 100);
	l = add_list(&u, 5);
	h_eq_i64("police par defaut : ligne 11 + 2 marges",
		ctl_lst_item_rect(l, 0).h, 11 + 2 * pad);
	ctl_set_font(&u.r, l, FONT_INS_BOLD_24);
	h_eq_i64("police 24 : ligne 24 + 2 marges", ctl_lst_item_rect(l, 0).h,
		24 + 2 * pad);
	h_eq_i64("police 24 : deuxieme ligne en dessous",
		ctl_lst_item_rect(l, 1).y,
		ctl_lst_item_rect(l, 0).y + 24 + 2 * pad);
	fake_done(&u);
}

static void	list_text_and_hit_follow_font(void)
{
	t_fakeui	u;
	t_ctl		*l;
	int32_t		row_h;

	row_h = 24 + 2 * CTL_ROW_PAD;
	fake_begin(&u, 200, 100);
	l = add_list(&u, 5);
	ctl_set_font(&u.r, l, FONT_INS_BOLD_24);
	ctl_invalidate(&u.r, l);
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("texte de ligne en police 24", fake_log_last(FK_TEXT)->r.h, 24);
	fake_click(&u.r, 20, 10 + CTL_FRAME + row_h + 3);
	h_eq_i64("clic dans la deuxieme ligne en police 24",
		ctl_list_selected(l), 1);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edge-font");
	h_run("liste : hauteur de ligne selon la police du controle",
		list_rows_follow_font);
	h_run("liste : texte et clic selon la police du controle",
		list_text_and_hit_follow_font);
	return (h_end());
}
