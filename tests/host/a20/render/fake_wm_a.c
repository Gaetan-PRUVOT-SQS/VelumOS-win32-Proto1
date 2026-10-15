#include <stdlib.h>
#include "velum/err.h"
#include "velum/libk.h"
#include "render.h"

t_fakewm	g_fwm;

int	wmc_connect(t_wmhello *info)
{
	memset(info, 0, sizeof(*info));
	info->version = WM_VERSION;
	info->screen_w = SCREEN_W;
	info->screen_h = SCREEN_H;
	info->workarea = rect_make(0, 0, SCREEN_W, SCREEN_H);
	return (0);
}

static t_rect	client_of(const t_wmcreate *rq)
{
	t_lunawin	lw;

	if (rq->style & (WS_DESKTOP | WS_FULLSCREEN))
		return (rect_make(0, 0, SCREEN_W, SCREEN_H));
	if (!(rq->style & WS_CAPTION) || (rq->style & (WS_APPBAR | WS_POPUP)))
		return (rect_make(0, 0, rq->rect.w, rq->rect.h));
	memset(&lw, 0, sizeof(lw));
	lw.outer = rq->rect;
	lw.style = rq->style;
	lw.title = rq->title;
	return (luna_window_client(&lw));
}

static void	fill_record(t_fwin *f, const t_wmcreate *rq, uint32_t id)
{
	f->id = id;
	f->style = rq->style;
	f->state = rq->state;
	f->rect = rq->rect;
	if (rq->style & (WS_DESKTOP | WS_FULLSCREEN))
		f->rect = rect_make(0, 0, SCREEN_W, SCREEN_H);
	strlcpy(f->title, rq->title, sizeof(f->title));
}

int	wmc_create(t_wmwin *w, const t_wmcreate *rq)
{
	t_fwin	*f;
	t_rect	c;

	if (g_fwm.count >= FWM_MAX)
		return (E_NOSPC);
	c = client_of(rq);
	f = &g_fwm.win[g_fwm.count];
	g_fwm.count++;
	fill_record(f, rq, g_fwm.next_id);
	g_fwm.next_id++;
	memset(w, 0, sizeof(*w));
	w->id = f->id;
	w->chan = 1;
	w->section = 1;
	w->style = rq->style;
	w->surface.px = calloc((size_t)c.w * (size_t)c.h, sizeof(uint32_t));
	gfx_surface_init(&w->surface, w->surface.px, c.w, c.h);
	f->w = w;
	if (rq->state != WSTATE_HIDDEN)
		g_fwm.activated = f->id;
	return (0);
}

int	wmc_destroy(t_wmwin *w)
{
	t_fwin	*f;

	f = fwm_by_id(w->id);
	if (f)
	{
		f->w = NULL;
		f->id = 0;
	}
	free(w->surface.px);
	memset(&w->surface, 0, sizeof(w->surface));
	w->id = 0;
	return (0);
}
