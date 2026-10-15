#include "fake.h"
#include "ctl_int.h"

t_rect	fake_mrect(const t_ctl *m, int lvl, int idx)
{
	return (ctl_mnu_item_rect(m->priv, (uint32_t)lvl, (uint32_t)idx));
}

t_point	fake_mpt(const t_ctl *m, int lvl, int idx)
{
	t_rect	r;

	r = fake_mrect(m, lvl, idx);
	return ((t_point){r.x + r.w / 2, r.y + r.h / 2});
}

bool	fk_dn(t_fakeui *u, const t_ctl *m, int lvl, int idx)
{
	t_point	p;
	bool	used;

	p = fake_mpt(m, lvl, idx);
	used = fake_down(&u->r, p.x, p.y);
	fake_up(&u->r, p.x, p.y);
	return (used);
}

bool	fk_mv(t_fakeui *u, const t_ctl *m, int lvl, int idx)
{
	t_point	p;

	p = fake_mpt(m, lvl, idx);
	return (fake_move(&u->r, p.x, p.y));
}

t_ctl	*fake_chain(t_fakeui *u, t_ctlmenuitem *a, int n)
{
	t_ctlmenuspec	ms;
	int				i;

	i = 0;
	while (i < n)
	{
		fake_item(&a[i], "x", (uint32_t)i, 0);
		if (i + 1 < n)
		{
			a[i].sub = &a[i + 1];
			a[i].nsub = 1;
		}
		i++;
	}
	ms.id = 100;
	ms.at = (t_point){10, 10};
	ms.items = a;
	ms.count = 1;
	return (ctl_menu_popup(&u->r, &ms));
}
