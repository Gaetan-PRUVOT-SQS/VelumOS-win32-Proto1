#include <stdio.h>
#include "fake.h"

t_ctl	*fake_list(t_fakeui *u, int items)
{
	t_ctl	*l;
	char	name[32];
	int		i;

	l = fake_add(u, fake_spec(CT_LIST, 1, rect_make(5, 5, 100, 60), ""));
	if (!l)
		return (NULL);
	l->cb = fake_cb;
	ctl_focus(&u->r, l);
	i = 0;
	while (i < items)
	{
		snprintf(name, sizeof(name), "item %d", i);
		ctl_list_add(&u->r, l, name);
		i++;
	}
	ctl_paint(&u->r);
	fake_cmd_reset();
	return (l);
}
