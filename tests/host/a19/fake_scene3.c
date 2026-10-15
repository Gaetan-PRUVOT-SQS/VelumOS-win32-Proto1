#include "fake.h"

t_ctl	*fake_btn(t_fakeui *u)
{
	t_ctl	*b;

	b = fake_add(u, fake_spec(CT_BUTTON, 1, rect_make(10, 10, 60, 20), "OK"));
	if (b)
		b->cb = fake_cb;
	return (b);
}

t_ctl	*fk_c(t_fakeui *u, uint32_t type, uint32_t id, t_rect r)
{
	return (fake_add(u, fake_spec(type, id, r, "")));
}

t_ctl	*fake_bar(t_fakeui *u, int max, int page)
{
	t_ctl	*s;

	s = fake_add(u, fake_spec(CT_SCROLL, 1, rect_make(100, 10, 17, 100), ""));
	s->cb = fake_cb;
	ctl_scroll_set(&u->r, s, max, page);
	ctl_paint(&u->r);
	fake_cmd_reset();
	return (s);
}
