#include <string.h>
#include "fake.h"
#include "ctl_int.h"

static const uint32_t	g_ids[14] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 21, 22,
	100};
static const uint32_t	g_fl[8] = {CTL_VISIBLE, CTL_ENABLED, CTL_FOCUSABLE,
	CTL_CHECKED, CTL_DEFAULT, CTL_PASSWORD, CTL_ALIGN_CENTER, CTL_ELLIPSIS};

static int	rnd(int lo, int hi)
{
	return (lo + (int)(fake_rand() % (uint32_t)(hi - lo)));
}

static void	storm_list(t_fakeui *u, t_ctl *c)
{
	uint32_t	k;

	k = fake_rand() % 8;
	if (k < 3)
		ctl_list_add(&u->r, c, "element &&");
	else if (k == 3)
		ctl_list_remove(&u->r, c, rnd(-2, 6));
	else if (k == 4)
		ctl_list_select(&u->r, c, rnd(-2, 6));
	else if (k == 5)
		ctl_list_clear(&u->r, c);
	else if (k == 6)
		ctl_scroll_set(&u->r, c, rnd(-5, 40), rnd(-5, 12));
	else
		ctl_scroll_move(&u->r, c, rnd(-5, 50));
}

static void	storm_shape(t_fakeui *u, t_ctl *c)
{
	uint32_t	k;

	k = fake_rand() % 6;
	if (k < 2)
		ctl_set_rect(&u->r, c, rect_make(rnd(-30, 320), rnd(-30, 260),
				rnd(0, 200), rnd(0, 90)));
	else if (k == 2)
		ctl_set_text(&u->r, c, "&Texte \xc3\xa9\xe2\x82\xac&");
	else if (k == 3)
		ctl_progress_set(&u->r, c, (uint32_t)rnd(0, 140));
	else if (k == 4 && fake_rand() % 8 == 0)
		ctl_remove(&u->r, c);
	else
		fake_mem_arm(rnd(1, 4));
}

void	fake_storm_props(t_fakeui *u)
{
	t_ctl	*c;

	c = ctl_find(&u->r, g_ids[fake_rand() % 14]);
	if (!c && fake_rand() % 4 == 0)
		fk_c(u, (uint32_t)rnd(0, CT_IMAGE), g_ids[fake_rand() % 14],
			rect_make(rnd(0, 250), rnd(0, 200), rnd(0, 100), rnd(0, 40)));
	else if (fake_rand() % 3 == 0)
		storm_list(u, c);
	else if (fake_rand() % 2 == 0)
		storm_shape(u, c);
	else
		ctl_set_flag(&u->r, c, g_fl[fake_rand() % 8], fake_rand() & 1);
}

bool	fake_storm_ok(const t_fakeui *u)
{
	const t_ctl	*c;

	c = ctl_walk_next(u->r.root);
	while (c)
	{
		if (!c->parent)
			return (false);
		c = ctl_walk_next(c);
	}
	return (true);
}
