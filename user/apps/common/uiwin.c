#include "velum/err.h"
#include "velum/libk.h"
#include "uiwin.h"

void	ui_request(t_wmcreate *rq, t_rect r, uint32_t style, const char *title)
{
	memset(rq, 0, sizeof(*rq));
	rq->rect = r;
	rq->style = style;
	rq->state = WSTATE_NORMAL;
	strlcpy(rq->title, title, sizeof(rq->title));
}

int	uiwin_open(t_uiwin *u, const t_wmcreate *rq, t_ctlcb command)
{
	int	r;

	memset(u, 0, sizeof(*u));
	r = wmc_create(&u->win, rq);
	if (r < 0)
		return (r);
	u->area = rect_make(0, 0, u->win.surface.w, u->win.surface.h);
	u->view = gfx_surface_sub(&u->win.surface, u->area);
	r = ctl_root_init(&u->root, &u->view, command);
	if (r < 0)
	{
		wmc_destroy(&u->win);
		return (r);
	}
	u->ready = 1;
	return (0);
}

int	uiwin_area(t_uiwin *u, t_rect area)
{
	t_rect	bound;

	bound = rect_make(0, 0, u->win.surface.w, u->win.surface.h);
	area = rect_intersect(area, bound);
	if (!u->ready || rect_empty(area))
		return (E_INVAL);
	u->area = area;
	u->view = gfx_surface_sub(&u->win.surface, area);
	ctl_set_rect(&u->root, u->root.root, rect_make(0, 0, area.w, area.h));
	ctl_invalidate(&u->root, u->root.root);
	return (0);
}

void	uiwin_close(t_uiwin *u)
{
	if (!u->ready)
		return ;
	ctl_root_destroy(&u->root);
	wmc_destroy(&u->win);
	u->ready = 0;
}

void	uiwin_flush(t_uiwin *u)
{
	t_rect	r;
	t_rect	pending;

	if (!u->ready)
		return ;
	if (u->underlay && !region_empty(&u->root.dirty))
	{
		pending = region_bounds(&u->root.dirty);
		pending.x += u->area.x;
		pending.y += u->area.y;
		u->underlay(u->underlay_user, pending);
	}
	r = ctl_paint(&u->root);
	if (rect_empty(r))
		return ;
	r.x += u->area.x;
	r.y += u->area.y;
	if (u->overlay)
		u->overlay(u->overlay_user, r);
	wmc_present(&u->win, &r, 1);
}
