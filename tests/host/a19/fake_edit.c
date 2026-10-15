#include "fake.h"

t_ctl	*fake_edit(t_fakeui *u, const char *text)
{
	t_ctl	*e;

	e = fake_add(u, fake_spec(CT_EDIT, 1, rect_make(5, 5, 150, 20), text));
	if (!e)
		return (NULL);
	e->cb = fake_cb;
	ctl_focus(&u->r, e);
	return (e);
}

void	fake_keys(t_ctlroot *r, uint32_t code, uint32_t mods, int times)
{
	while (times > 0)
	{
		fake_press(r, code, mods);
		times--;
	}
}
