#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	click_and_toggle(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_rect		want;

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	want = c[0]->rect;
	fake_move(&u.r, 15, 15);
	ctl_paint(&u.r);
	fake_down(&u.r, 15, 15);
	h_true(fake_dirty_is(&u.r, &want, 1), "appui : rect du bouton");
	ctl_paint(&u.r);
	fake_up(&u.r, 15, 15);
	h_true(fake_dirty_is(&u.r, &want, 1), "relachement : rect du bouton");
	ctl_paint(&u.r);
	ctl_focus(&u.r, c[3]);
	ctl_paint(&u.r);
	fake_press(&u.r, VK_SPACE, 0);
	want = c[3]->rect;
	h_true(fake_dirty_is(&u.r, &want, 1), "case cochee : rect de la case");
	fake_done(&u);
}

static void	add_remove_clip(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_ctl		*n;
	t_rect		want;

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	n = fake_add(&u, fake_spec(CT_BUTTON, 9, rect_make(190, 110, 50, 50), "n"));
	want = rect_make(190, 110, 10, 10);
	h_true(fake_dirty_is(&u.r, &want, 1), "ajout ecrete a la surface");
	ctl_paint(&u.r);
	ctl_remove(&u.r, n);
	h_true(fake_dirty_is(&u.r, &want, 1), "retrait ecrete a la surface");
	ctl_paint(&u.r);
	fake_add(&u, fake_spec(CT_BUTTON, 10, rect_make(300, 300, 5, 5), "h"));
	h_true(fake_dirty_is(&u.r, NULL, 0), "controle hors surface : rien");
	fake_done(&u);
}

static void	overflow_keeps_coverage(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_rect		r;
	int			i;

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	i = 0;
	while (i < 40)
	{
		r = rect_make(i * 4, (i % 3) * 30, 3, 3);
		ctl_set_rect(&u.r, c[1], r);
		h_true(fake_dirty_covers(&u.r, r), "chaque zone reste couverte");
		i++;
	}
	h_true(u.r.dirty.n <= GFX_REGION_MAX, "region bornee");
	h_true(fake_dirty_covers(&u.r, rect_make(0, 0, 3, 3)), "la premiere aussi");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/dirty2");
	h_run("clic et case : zones exactes", click_and_toggle);
	h_run("ajout et retrait ecretes a la surface", add_remove_clip);
	h_run("debordement de region : couverture", overflow_keeps_coverage);
	return (h_end());
}
