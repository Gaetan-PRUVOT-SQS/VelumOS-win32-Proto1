#include "ctl_int.h"

int	ctl_root_init(t_ctlroot *r, t_surface *surface, t_ctlcb command)
{
	t_ctlspec	spec;

	if (!r || !surface)
		return (E_INVAL);
	memset(r, 0, sizeof(*r));
	spec.type = CT_PANEL;
	spec.id = 0;
	spec.flags = CTL_VISIBLE | CTL_ENABLED;
	spec.rect = rect_make(0, 0, surface->w, surface->h);
	spec.text = NULL;
	if (!ctl_rect_ok(spec.rect))
		return (E_INVAL);
	r->root = ctl_make(&spec, sizeof(t_rootpriv));
	if (!r->root)
		return (E_NOMEM);
	r->surface = surface;
	r->command = command;
	ctl_dirty_add(r, spec.rect);
	return (0);
}

void	ctl_root_destroy(t_ctlroot *r)
{
	if (!r || !r->root)
		return ;
	ctl_free_subtree(r->root);
	memset(r, 0, sizeof(*r));
}

void	ctl_set_clock(t_ctlroot *r, t_ctlclock now)
{
	if (!r || !r->root)
		return ;
	ctl_rootpriv(r)->clock = now;
}
