#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

void	fake_dscene(t_fakeui *u, t_ctl **c)
{
	c[0] = fake_add(u, fake_spec(CT_BUTTON, 1, rect_make(10, 10, 60, 20), "B"));
	c[1] = fake_add(u, fake_spec(CT_LABEL, 2, rect_make(10, 40, 80, 12), "L"));
	c[2] = fake_add(u, fake_spec(CT_EDIT, 3, rect_make(10, 60, 100, 20), "E"));
	c[3] = fake_add(u, fake_spec(CT_CHECK, 4, rect_make(10, 90, 80, 14), "C"));
	ctl_paint(&u->r);
	fake_log_clear();
}

bool	fk_no(t_fakeui *u, t_ctlspec sp)
{
	int		live;
	t_ctl	*c;

	live = fake_mem_live();
	c = ctl_add(&u->r, NULL, &sp);
	return (c == NULL && fake_mem_live() == live);
}

static bool	linked(const t_ctl *c)
{
	const t_ctl	*k;

	k = c->parent->first;
	while (k && k != c)
		k = k->next;
	return (k == c && ctl_depth(c) < CTL_DEPTH_MAX);
}

void	fake_check_tree(t_fakeui *u, int items)
{
	const t_ctl	*c;
	const t_ctl	*l;
	bool		sound;

	sound = true;
	c = ctl_walk_next(u->r.root);
	while (c)
	{
		sound = sound && linked(c);
		c = ctl_walk_next(c);
	}
	h_true(sound, "arbre coherent apres un echec d'allocation");
	l = ctl_find(&u->r, 9);
	if (l)
		h_eq_i64("liste complete", ctl_list_count(l), items);
}
