#include <stdlib.h>
#include "help.h"

#define POISON 0x00123456u

static void	poison(t_surface *fb)
{
	int32_t	i;

	i = 0;
	while (i < fb->stride * fb->h)
	{
		fb->px[i] = POISON;
		i++;
	}
}

static int	count_bad(const t_wsrv *s, const t_surface *ref, t_rect in)
{
	t_point		p;
	uint32_t	got;
	uint32_t	want;
	int			bad;

	bad = 0;
	p.y = 0;
	while (p.y < ref->h)
	{
		p.x = 0;
		while (p.x < ref->w)
		{
			got = s->fb.px[p.y * s->fb.stride + p.x] & 0xffffff;
			want = POISON;
			if (rect_contains(in, p))
				want = ref->px[p.y * ref->stride + p.x] & 0xffffff;
			bad += (got != want);
			p.x++;
		}
		p.y++;
	}
	return (bad);
}

int	hc_frame(t_wsrv *s, t_rect dirty, t_rect in)
{
	t_surface	ref;
	uint32_t	*px;
	int			bad;

	px = calloc((size_t)s->t.screen.w * s->t.screen.h, 4);
	gfx_surface_init(&ref, px, s->t.screen.w, s->t.screen.h);
	ws_mark(s, s->dirty.screen);
	ws_frame(s);
	poison(&s->fb);
	ws_mark(s, dirty);
	ws_frame(s);
	wc_compose(&s->t, &ref, s->cur, s->t.screen);
	bad = count_bad(s, &ref, in);
	free(px);
	return (bad);
}
