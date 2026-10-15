#include <stdlib.h>
#include "fake.h"

int	fake_surf(t_surface *s, int w, int h)
{
	int	i;

	s->px = malloc(((size_t)w * (size_t)h + 1) * sizeof(uint32_t));
	if (!s->px)
		return (-1);
	i = 0;
	while (i < w * h)
	{
		s->px[i] = SENTINEL;
		i++;
	}
	s->w = w;
	s->h = h;
	s->stride = w;
	s->clip = rect_make(0, 0, w, h);
	return (0);
}

void	fake_surf_free(t_surface *s)
{
	free(s->px);
	s->px = NULL;
}

int	fake_ui_open(t_fakeui *u, int w, int h)
{
	int	rc;

	if (fake_surf(&u->s, w, h) != 0)
		return (-1);
	fake_log_clear();
	fake_cmd_reset();
	fake_clock_set(0);
	g_fake_buttons = 0;
	g_fake_wheel = 0;
	g_fake_picked = -1;
	rc = ctl_root_init(&u->r, &u->s, fake_cmd);
	if (rc != 0)
		fake_surf_free(&u->s);
	return (rc);
}

void	fake_ui_close(t_fakeui *u)
{
	ctl_root_destroy(&u->r);
	fake_surf_free(&u->s);
}

t_ctlspec	fake_spec(uint32_t type, uint32_t id, t_rect r, const char *text)
{
	t_ctlspec	sp;

	sp.type = type;
	sp.id = id;
	sp.rect = r;
	sp.text = text;
	sp.flags = CTL_VISIBLE | CTL_ENABLED;
	if (type == CT_BUTTON || type == CT_CHECK || type == CT_RADIO
		|| type == CT_EDIT || type == CT_LIST || type == CT_SCROLL)
		sp.flags |= CTL_FOCUSABLE;
	return (sp);
}
