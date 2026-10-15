#include "fake.h"

static const uint32_t	g_vk[20] = {VK_TAB, VK_SPACE, VK_RETURN, VK_ESCAPE,
	VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_HOME, VK_END, VK_PRIOR, VK_NEXT,
	VK_BACK, VK_DELETE, 'A', 'C', 'V', 'X', 'N', 'O'};
static const uint32_t	g_mods[6] = {0, INPM_SHIFT, INPM_CTRL, INPM_ALT,
	INPM_CTRL | INPM_ALT, INPM_REPEAT};

static int	rnd(int lo, int hi)
{
	return (lo + (int)(fake_rand() % (uint32_t)(hi - lo)));
}

static void	storm_mouse(t_fakeui *u)
{
	int	x;
	int	y;

	x = rnd(-10, 330);
	y = rnd(-10, 270);
	if (fake_rand() % 4 == 0)
		fake_down(&u->r, x, y);
	else if (fake_rand() % 3 == 0)
		fake_up(&u->r, x, y);
	else if (fake_rand() % 2 == 0)
		fake_wheel(&u->r, x, y, rnd(-5, 5));
	else
		fake_move(&u->r, x, y);
}

static void	storm_key(t_fakeui *u)
{
	if (fake_rand() % 4 == 0)
		fake_char(&u->r, fake_rand() % 0x1100);
	else
		fake_press(&u->r, g_vk[fake_rand() % 20], g_mods[fake_rand() % 6]);
}

static void	storm_menu(t_fakeui *u)
{
	t_ctl	*m;

	m = ctl_find(&u->r, 100);
	if (m && fake_rand() % 3 == 0)
		ctl_menu_close(&u->r, m);
	else
		fake_menu(u, rnd(-20, 330), rnd(-20, 270));
}

void	fake_storm_step(t_fakeui *u)
{
	uint32_t	k;

	k = fake_rand() % 16;
	if (k < 7)
		storm_mouse(u);
	else if (k < 12)
		storm_key(u);
	else if (k == 12)
		storm_menu(u);
	else if (k == 13)
		ctl_paint(&u->r);
	else
		fake_storm_props(u);
}
