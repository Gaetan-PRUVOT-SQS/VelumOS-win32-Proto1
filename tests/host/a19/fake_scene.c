#include "fake.h"

void	fake_item(t_ctlmenuitem *it, const char *text, uint32_t id, uint32_t f)
{
	it->text = text;
	it->id = id;
	it->flags = f;
	it->sub = NULL;
	it->nsub = 0;
}

t_ctl	*fake_scene_menu(t_fakeui *u)
{
	t_ctlmenuitem	sub[2];
	t_ctlmenuitem	top[3];
	t_ctlmenuspec	ms;

	fake_item(&sub[0], "&Un", 21, 0);
	fake_item(&sub[1], "&Deux", 22, 0);
	fake_item(&top[0], "&Ouvrir", 20, 0);
	fake_item(&top[1], "", 0, CTL_MI_SEPARATOR);
	fake_item(&top[2], "&Plus", 23, 0);
	top[2].sub = sub;
	top[2].nsub = 2;
	ms.id = 12;
	ms.at = (t_point){20, 20};
	ms.items = top;
	ms.count = 3;
	return (ctl_menu_popup(&u->r, &ms));
}

static void	scene_basic(t_fakeui *u, t_ctl *p)
{
	t_ctl	*g;

	fake_addp(u, p, fake_spec(CT_LABEL, 2, rect_make(5, 5, 60, 12), "&Nom"));
	fake_addp(u, p, fake_spec(CT_BUTTON, 3, rect_make(5, 20, 60, 22), "&OK"));
	fake_addp(u, p, fake_spec(CT_CHECK, 4, rect_make(5, 45, 80, 14), "&Gras"));
	g = fake_addp(u, p, fake_spec(CT_GROUP, 7, rect_make(70, 5, 70, 60), "G"));
	fake_addp(u, g, fake_spec(CT_RADIO, 5, rect_make(75, 20, 60, 14), "&A"));
	fake_addp(u, g, fake_spec(CT_RADIO, 6, rect_make(75, 38, 60, 14), "&B"));
}

static int	scene_list(t_fakeui *u, t_ctl *p)
{
	t_ctl	*l;
	int		i;
	int		ok;

	l = fake_addp(u, p, fake_spec(CT_LIST, 9, rect_make(5, 70, 100, 50), ""));
	ok = 0;
	i = 0;
	while (l && i < 20)
	{
		if (ctl_list_add(&u->r, l, "element") >= 0)
			ok++;
		i++;
	}
	return (ok);
}

int	fake_scene(t_fakeui *u)
{
	t_ctl	*p;
	int		ok;

	p = fake_add(u, fake_spec(CT_PANEL, 1, rect_make(0, 0, 200, 150), ""));
	if (!p)
		return (0);
	scene_basic(u, p);
	fake_addp(u, p, fake_spec(CT_EDIT, 8, rect_make(110, 5, 80, 20), "texte"));
	ok = scene_list(u, p);
	fake_addp(u, p, fake_spec(CT_SCROLL, 10, rect_make(180, 40, 17, 80), ""));
	fake_addp(u, p, fake_spec(CT_PROGRESS, 11, rect_make(110, 40, 60, 14), ""));
	fake_scene_menu(u);
	return (ok);
}
