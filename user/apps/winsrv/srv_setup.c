#include "velum/err.h"
#include "velum/libk.h"
#include "ws_srv.h"

void	ws_setup(t_wsrv *s, t_rect screen)
{
	memset(s, 0, sizeof(*s));
	wt_init(&s->t, screen);
	wdy_init(&s->dirty, screen);
	s->m.pos.x = screen.x + screen.w / 2;
	s->m.pos.y = screen.y + screen.h / 2;
	s->m.capture = -1;
	s->m.hover = -1;
	s->m.hot_slot = -1;
	s->m.press_slot = -1;
	s->m.click_slot = -1;
	s->drag.slot = -1;
	s->shell = -1;
	s->cur.pos = s->m.pos;
	s->cur.shape = CUR_ARROW;
	s->cur.visible = true;
	ws_mark(s, screen);
}

static bool	geometry_ok(const t_dispinfo *d)
{
	return (d->bpp == 32 && d->width >= 1 && d->height >= 1
		&& d->width <= WS_DIM_MAX && d->height <= WS_DIM_MAX
		&& d->pitch % 4 == 0 && d->pitch >= d->width * 4
		&& (d->fb_size == 0
			|| (uint64_t)d->pitch * d->height <= d->fb_size));
}

static int	open_screen(t_wsrv *s)
{
	t_dispinfo	di;
	void		*fb;
	uint32_t	*back;

	fb = NULL;
	if (ws_sys_display(&di, &fb) < 0 || fb == NULL || !geometry_ok(&di))
		return (E_NODEV);
	back = ws_sys_alloc((uint64_t)di.width * di.height * 4);
	if (back == NULL)
		return (E_NOMEM);
	if (ws_sys_kcon(0) < 0)
		ws_sys_log("winsrv: console noyau toujours active");
	ws_setup(s, rect_make(0, 0, (int32_t)di.width, (int32_t)di.height));
	gfx_surface_init(&s->back, back, (int32_t)di.width, (int32_t)di.height);
	gfx_surface_init(&s->fb, fb, (int32_t)di.width, (int32_t)di.height);
	s->fb.stride = (int32_t)(di.pitch / 4);
	ws_glass_open(s);
	return (0);
}

int	ws_open(t_wsrv *s)
{
	int64_t	h;
	int		r;

	r = open_screen(s);
	if (r < 0)
		return (r);
	h = ws_sys_listen(WM_PORT_NAME);
	if (h <= 0)
	{
		ws_close_all(s);
		if (h == 0)
			return (E_IO);
		return ((int)h);
	}
	s->listener = (t_handle)h;
	h = ws_sys_input_open();
	if (h > 0)
		s->input = (t_handle)h;
	else
		ws_sys_log("winsrv: entree indisponible");
	ws_sys_log("winsrv: pret");
	return (0);
}

void	ws_close_all(t_wsrv *s)
{
	int	i;

	i = 0;
	while (i < WS_CLIENT_MAX)
	{
		if (s->c[i].flags & WCF_USED)
			ws_client_drop(s, i);
		i++;
	}
	if (s->listener != 0)
		ws_sys_close(s->listener);
	if (s->input != 0)
		ws_sys_close(s->input);
	ws_glass_close(s);
	if (s->back.px != NULL)
		ws_sys_free(s->back.px, (uint64_t)s->back.w * s->back.h * 4);
	s->back.px = NULL;
	s->listener = 0;
	s->input = 0;
}
