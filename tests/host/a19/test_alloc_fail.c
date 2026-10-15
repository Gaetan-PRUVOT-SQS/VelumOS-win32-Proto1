#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	sweep_scene(void)
{
	t_fakeui	u;
	int			total;
	int			k;
	int			items;

	fake_mem_reset(0);
	fake_ui_open(&u, 200, 150);
	fake_scene(&u);
	fake_ui_close(&u);
	total = fake_mem_calls();
	h_true(total > 30, "la scene fait des dizaines d'allocations");
	k = 1;
	while (k <= total)
	{
		fake_mem_reset(k);
		if (fake_ui_open(&u, 200, 150) == 0)
		{
			items = fake_scene(&u);
			fake_check_tree(&u, items);
			fake_ui_close(&u);
		}
		h_eq_i64("aucune fuite", fake_mem_live(), 0);
		h_eq_i64("un seul echec injecte", fake_mem_failed(), 1);
		k++;
	}
}

static void	sweep_list(void)
{
	t_fakeui	u;
	t_ctl		*l;
	int			k;
	int			i;
	int			ok;

	k = 1;
	while (k <= 100)
	{
		fake_mem_reset(0);
		fake_ui_open(&u, 100, 100);
		l = fake_add(&u, fake_spec(CT_LIST, 1, rect_make(0, 0, 80, 60), ""));
		fake_mem_arm(k);
		ok = 0;
		i = 0;
		while (i++ < 40)
			ok += (ctl_list_add(&u.r, l, "x") >= 0);
		h_eq_i64("ajouts reussis = elements", ctl_list_count(l), ok);
		fake_ui_close(&u);
		h_eq_i64("aucune fuite", fake_mem_live(), 0);
		k++;
	}
}

static void	popup_failure(void)
{
	t_fakeui	u;
	t_ctl		*b;
	int			before;

	fake_begin(&u, 200, 150);
	b = fake_add(&u, fake_spec(CT_BUTTON, 1, rect_make(0, 0, 50, 20), "b"));
	ctl_focus(&u.r, b);
	before = fake_mem_live();
	fake_mem_arm(1);
	h_true(fake_scene_menu(&u) == NULL, "popup refuse si l'allocation echoue");
	h_eq_i64("rien d'alloue", fake_mem_live(), before);
	h_true(u.r.focus == b && u.r.capture == NULL, "etat inchange");
	h_true(u.r.root->first == b, "arbre intact");
	h_true(b->next == NULL, "pas de menu ajoute");
	fake_mem_arm(0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/alloc_fail");
	h_run("scene complete : chaque allocation echoue une fois", sweep_scene);
	h_run("liste : echec a chaque point de croissance", sweep_list);
	h_run("popup : echec sans effet de bord", popup_failure);
	return (h_end());
}
