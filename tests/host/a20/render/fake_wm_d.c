#include "velum/libk.h"
#include "render.h"

int	wmc_present(t_wmwin *w, const t_rect *rects, uint32_t n)
{
	t_fwin	*f;

	(void)rects;
	(void)n;
	f = fwm_by_id(w->id);
	if (f)
		f->presents++;
	return (0);
}

static int	band_of(const t_fwin *f)
{
	if (f->style & WS_DESKTOP)
		return (0);
	if (f->style & (WS_TOPMOST | WS_APPBAR | WS_POPUP))
		return (2);
	return (1);
}

static void	paint_win(t_surface *screen, const t_fwin *f)
{
	t_lunawin	lw;
	t_rect		cl;
	t_blit		b;

	if (!f->w || f->state == WSTATE_HIDDEN || f->state == WSTATE_MIN)
		return ;
	cl = f->rect;
	if ((f->style & WS_CAPTION) && !(f->style & (WS_APPBAR | WS_POPUP
				| WS_DESKTOP | WS_FULLSCREEN)))
	{
		memset(&lw, 0, sizeof(lw));
		lw.outer = f->rect;
		lw.style = f->style;
		lw.title = f->title;
		lw.active = (f->id == g_fwm.activated);
		luna_window_frame(screen, &lw);
		cl = luna_window_client(&lw);
	}
	b.dst = screen;
	b.src = &f->w->surface;
	b.dr = rect_make(cl.x, cl.y, f->w->surface.w, f->w->surface.h);
	b.sr = rect_make(0, 0, f->w->surface.w, f->w->surface.h);
	gfx_blit(&b);
}

void	fwm_compose(t_surface *screen)
{
	int			band;
	uint32_t	i;

	gfx_fill(screen, rect_make(0, 0, screen->w, screen->h), 0xff000000);
	band = 0;
	while (band <= 2)
	{
		i = 0;
		while (i < g_fwm.count)
		{
			if (band_of(&g_fwm.win[i]) == band)
				paint_win(screen, &g_fwm.win[i]);
			i++;
		}
		band++;
	}
}
