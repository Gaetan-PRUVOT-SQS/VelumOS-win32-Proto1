#include "fake.h"

static void	dialog_top(t_fakeui *u)
{
	t_ctl	*pw;

	fake_add(u, fake_spec(CT_LABEL, 21, rect_make(10, 10, 50, 14), "&Nom"));
	fk_c(u, CT_EDIT, 3, rect_make(70, 8, 150, 20));
	fake_add(u, fake_spec(CT_LABEL, 22, rect_make(10, 40, 50, 14), "&Secret"));
	pw = fk_c(u, CT_EDIT, 4, rect_make(70, 38, 150, 20));
	ctl_set_flag(&u->r, pw, CTL_PASSWORD, true);
	fake_add(u, fake_spec(CT_CHECK, 5, rect_make(10, 70, 100, 14), "&Options"));
}

static void	dialog_group(t_fakeui *u)
{
	t_ctl	*g;

	g = fake_add(u, fake_spec(CT_GROUP, 6, rect_make(10, 90, 200, 60),
				"Choix"));
	fake_addp(u, g, fake_spec(CT_RADIO, 7, rect_make(20, 105, 50, 14), "&A"));
	fake_addp(u, g, fake_spec(CT_RADIO, 8, rect_make(20, 122, 50, 14), "&B"));
	fake_addp(u, g, fake_spec(CT_RADIO, 9, rect_make(20, 138, 50, 14), "&C"));
	ctl_set_flag(&u->r, ctl_find(&u->r, 8), CTL_CHECKED, true);
}

void	fake_dialog(t_fakeui *u)
{
	t_ctl	*l;
	t_ctl	*ok;

	dialog_top(u);
	dialog_group(u);
	l = fk_c(u, CT_LIST, 10, rect_make(10, 160, 150, 50));
	ctl_list_add(&u->r, l, "un");
	ctl_list_add(&u->r, l, "deux");
	ctl_list_add(&u->r, l, "trois");
	ctl_scroll_set(&u->r, fk_c(u, CT_SCROLL, 11, rect_make(170, 160, 17, 50)),
		10, 3);
	ok = fake_add(u, fake_spec(CT_BUTTON, CTL_ID_OK, rect_make(150, 225, 60,
					22), "OK"));
	ctl_set_flag(&u->r, ok, CTL_DEFAULT, true);
	fake_add(u, fake_spec(CT_BUTTON, CTL_ID_CANCEL, rect_make(220, 225, 70, 22),
			"&Annuler"));
}
