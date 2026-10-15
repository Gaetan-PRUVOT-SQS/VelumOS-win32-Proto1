#include "fake.h"

static const uint32_t	g_keys[14] = {VK_TAB, VK_SPACE, VK_RETURN, VK_ESCAPE,
	VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_HOME, VK_END, VK_PRIOR, VK_NEXT,
	VK_BACK, VK_DELETE};

static void	poke_mouse(t_fakeui *u, const t_ctl *c)
{
	int	x;
	int	y;

	x = c->rect.x;
	y = c->rect.y;
	fake_move(&u->r, x, y);
	fake_down(&u->r, x + c->rect.w / 2, y + c->rect.h / 2);
	fake_move(&u->r, x + c->rect.w - 1, y + c->rect.h - 1);
	fake_wheel(&u->r, x, y, 1);
	fake_up(&u->r, x + c->rect.w, y + c->rect.h);
	fake_click(&u->r, x + c->rect.w - 1, y + c->rect.h - 1);
	fake_click(&u->r, x, y);
}

void	fake_poke(t_fakeui *u, t_ctl *c)
{
	int	i;

	ctl_focus(&u->r, c);
	poke_mouse(u, c);
	i = 0;
	while (i < 14)
	{
		fake_press(&u->r, g_keys[i], 0);
		fake_press(&u->r, g_keys[i], INPM_SHIFT);
		i++;
	}
	fake_type(&u->r, "a\xc3\xa9\xe2\x82\xac");
	ctl_invalidate(&u->r, u->r.root);
	ctl_paint(&u->r);
}
