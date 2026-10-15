#include "fake.h"
#include "ctl_int.h"

static void	fill(t_ctlmenuitem *t, t_ctlmenuitem *s, t_ctlmenuitem *ss)
{
	fake_item(&ss[0], "&Delta", 531, 0);
	fake_item(&s[0], "&Alpha", 51, 0);
	fake_item(&s[1], "&Beta", 52, 0);
	fake_item(&s[2], "", 0, CTL_MI_SEPARATOR);
	fake_item(&s[3], "&Gamma", 53, 0);
	s[3].sub = ss;
	s[3].nsub = 1;
	fake_item(&t[0], "&Nouveau", 1, 0);
	fake_item(&t[1], "&Nord", 8, 0);
	fake_item(&t[2], "&Ouvrir", 2, 0);
	fake_item(&t[3], "", 0, CTL_MI_SEPARATOR);
	fake_item(&t[4], "&Fermer", 3, CTL_MI_DISABLED);
	fake_item(&t[5], "&Enregistrer", 4, CTL_MI_CHECKED);
	fake_item(&t[6], "&Plus", 5, 0);
	t[6].sub = s;
	t[6].nsub = 4;
	fake_item(&t[7], "", 0, CTL_MI_SEPARATOR);
	fake_item(&t[8], "&Quitter", 6, 0);
}

t_ctl	*fake_menu_id(t_fakeui *u, int x, int y, uint32_t id)
{
	t_ctlmenuitem	top[9];
	t_ctlmenuitem	sub[4];
	t_ctlmenuitem	subsub[1];
	t_ctlmenuspec	ms;
	t_ctl			*m;

	fill(top, sub, subsub);
	ms.id = id;
	ms.at = (t_point){x, y};
	ms.items = top;
	ms.count = 9;
	m = ctl_menu_popup(&u->r, &ms);
	if (m)
		m->cb = fake_cb_pick;
	return (m);
}

t_ctl	*fake_menu(t_fakeui *u, int x, int y)
{
	return (fake_menu_id(u, x, y, 100));
}

int	fake_msel(const t_ctl *m, int lvl)
{
	return (((const t_menu *)m->priv)->lv[lvl].sel);
}

int	fake_mdepth(const t_ctl *m)
{
	return ((int)((const t_menu *)m->priv)->depth);
}
