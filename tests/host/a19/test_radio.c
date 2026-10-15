#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	mkgroup(t_fakeui *u, t_ctl *p, uint32_t first)
{
	uint32_t	i;
	t_ctl		*r;

	i = 0;
	while (i < 3)
	{
		r = fake_addp(u, p, fake_spec(CT_RADIO, first + i,
					rect_make(p->rect.x + 10, 10 + 20 * (int)i, 80, 14), "&R"));
		r->cb = fake_cb;
		i++;
	}
}

static void	exclusivity_and_groups(void)
{
	t_fakeui	u;
	t_ctl		*p2;

	fake_begin(&u, 300, 100);
	mkgroup(&u, u.r.root, 1);
	p2 = fake_add(&u, fake_spec(CT_PANEL, 50, rect_make(150, 0, 100, 100), ""));
	mkgroup(&u, p2, 4);
	fake_click(&u.r, 12, 32);
	h_true(ctl_find(&u.r, 2)->flags & CTL_CHECKED, "R2 cochee");
	fake_click(&u.r, 12, 12);
	h_true(ctl_find(&u.r, 1)->flags & CTL_CHECKED, "R1 cochee");
	h_true(!(ctl_find(&u.r, 2)->flags & CTL_CHECKED), "R2 decochee");
	fake_click(&u.r, 162, 52);
	h_true(ctl_find(&u.r, 6)->flags & CTL_CHECKED, "autre groupe : R6 cochee");
	h_true(ctl_find(&u.r, 1)->flags & CTL_CHECKED, "groupe 1 intact");
	fake_click(&u.r, 12, 12);
	h_eq_i64("clic sur la radio deja cochee notifie", fake_cmd_find(1,
			CN_CLICKED), 2);
	fake_done(&u);
}

static void	arrows(void)
{
	t_fakeui	u;

	fake_begin(&u, 300, 100);
	mkgroup(&u, u.r.root, 1);
	fake_click(&u.r, 12, 12);
	h_true(fake_press(&u.r, VK_DOWN, 0), "Bas consomme");
	h_true(u.r.focus == ctl_find(&u.r, 2), "focus sur R2");
	h_true(ctl_find(&u.r, 2)->flags & CTL_CHECKED, "R2 cochee par la fleche");
	fake_press(&u.r, VK_RIGHT, 0);
	fake_press(&u.r, VK_RIGHT, 0);
	h_true(u.r.focus == ctl_find(&u.r, 1), "retour a R1 (cyclique)");
	fake_press(&u.r, VK_UP, 0);
	h_true(u.r.focus == ctl_find(&u.r, 3), "Haut depuis R1 : R3");
	h_eq_i64("une notification par selection", fake_cmd_find(3,
			CN_CLICKED), 2);
	ctl_set_flag(&u.r, ctl_find(&u.r, 2), CTL_ENABLED, false);
	fake_press(&u.r, VK_UP, 0);
	h_true(u.r.focus == ctl_find(&u.r, 1), "R2 desactivee sautee");
	fake_done(&u);
}

static void	set_flag_and_single(void)
{
	t_fakeui	u;

	fake_begin(&u, 300, 100);
	mkgroup(&u, u.r.root, 1);
	ctl_set_flag(&u.r, ctl_find(&u.r, 1), CTL_CHECKED, true);
	ctl_set_flag(&u.r, ctl_find(&u.r, 3), CTL_CHECKED, true);
	h_true(!(ctl_find(&u.r, 1)->flags & CTL_CHECKED), "exclusivite");
	ctl_set_flag(&u.r, ctl_find(&u.r, 3), CTL_CHECKED, false);
	h_true(!(ctl_find(&u.r, 3)->flags & CTL_CHECKED), "decocher est permis");
	ctl_remove(&u.r, ctl_find(&u.r, 1));
	ctl_remove(&u.r, ctl_find(&u.r, 3));
	ctl_focus(&u.r, ctl_find(&u.r, 2));
	h_true(fake_press(&u.r, VK_DOWN, 0), "radio seule : fleche consommee");
	h_true(u.r.focus == ctl_find(&u.r, 2), "focus inchange");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/radio");
	h_run("exclusivite et groupes independants", exclusivity_and_groups);
	h_run("fleches : cycle, notification, desactive saute", arrows);
	h_run("drapeau par programme, radio seule", set_flag_and_single);
	return (h_end());
}
