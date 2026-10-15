#include "fake.h"

t_ctl	*fake_add(t_fakeui *u, t_ctlspec spec)
{
	return (ctl_add(&u->r, NULL, &spec));
}

t_ctl	*fake_addp(t_fakeui *u, t_ctl *parent, t_ctlspec spec)
{
	return (ctl_add(&u->r, parent, &spec));
}
