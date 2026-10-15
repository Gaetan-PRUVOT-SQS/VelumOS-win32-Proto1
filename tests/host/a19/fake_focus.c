#include "fake.h"

void	fake_fscene(t_fakeui *u)
{
	t_ctl	*p;
	t_ctl	*g;

	fk_c(u, CT_BUTTON, 1, rect_make(0, 0, 40, 20));
	fk_c(u, CT_BUTTON, 2, rect_make(0, 25, 40, 20));
	fk_c(u, CT_BUTTON, 3, rect_make(0, 50, 40, 20));
	fk_c(u, CT_LABEL, 4, rect_make(0, 75, 40, 12));
	ctl_set_flag(&u->r, ctl_find(&u->r, 2), CTL_ENABLED, false);
	ctl_set_flag(&u->r, ctl_find(&u->r, 3), CTL_VISIBLE, false);
	p = fk_c(u, CT_PANEL, 5, rect_make(50, 0, 60, 30));
	fake_addp(u, p, fake_spec(CT_BUTTON, 6, rect_make(50, 0, 40, 20), ""));
	g = fk_c(u, CT_PANEL, 7, rect_make(120, 0, 60, 90));
	fake_addp(u, g, fake_spec(CT_RADIO, 8, rect_make(120, 0, 40, 14), ""));
	fake_addp(u, g, fake_spec(CT_RADIO, 9, rect_make(120, 20, 40, 14), ""));
	fake_addp(u, g, fake_spec(CT_RADIO, 10, rect_make(120, 40, 40, 14), ""));
	fk_c(u, CT_BUTTON, 11, rect_make(0, 100, 40, 20));
}

int	fake_tabs(t_fakeui *u, int *out, bool forward)
{
	int			n;
	uint32_t	mods;

	n = 0;
	mods = 0;
	if (!forward)
		mods = INPM_SHIFT;
	while (n < 8 && fake_press(&u->r, VK_TAB, mods))
	{
		if (n && (int)u->r.focus->id == out[0])
			break ;
		out[n] = (int)u->r.focus->id;
		n++;
	}
	return (n);
}

void	fake_apply(t_fakeui *u, const int (*m)[3])
{
	int	i;

	i = 0;
	while (i < 3)
	{
		if (m[i][0])
			ctl_set_flag(&u->r, ctl_find(&u->r, (uint32_t)m[i][0]),
				(uint32_t)m[i][1], m[i][2] != 0);
		i++;
	}
}
