#include "fake.h"

void	fake_cb_kill(void *c, uint32_t code, void *user)
{
	(void)code;
	ctl_remove((t_ctlroot *)user, c);
}

void	fake_cb_destroy(void *c, uint32_t code, void *user)
{
	(void)c;
	(void)code;
	ctl_root_destroy((t_ctlroot *)user);
}

int	g_fake_picked;

void	fake_cb_pick(void *c, uint32_t code, void *user)
{
	(void)user;
	if (code == CN_SELECT)
		g_fake_picked = ctl_menu_result(c);
}

void	fake_cb_repop(void *c, uint32_t code, void *user)
{
	t_fakeui	*u;

	(void)c;
	u = user;
	if (code == CN_SELECT)
		fake_menu(u, 30, 30);
}
