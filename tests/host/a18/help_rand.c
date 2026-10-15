#include <string.h>
#include "help.h"

t_rect	hx_rect(uint64_t *st)
{
	return (rect_make((int32_t)hg_below(st, 360) - 40,
			(int32_t)hg_below(st, 280) - 40, 1 + (int32_t)hg_below(st, 300),
			1 + (int32_t)hg_below(st, 220)));
}

int	hx_own(const t_wsrv *s, int ci, uint64_t *st)
{
	int	n;
	int	i;
	int	pick;

	n = (int)wt_count_owner(&s->t, ci);
	if (n == 0)
		return (-1);
	pick = (int)hg_below(st, (uint32_t)n);
	i = 0;
	while (i < WS_WIN_MAX)
	{
		if (s->t.w[i].id != 0 && s->t.w[i].owner == ci && pick-- == 0)
			return (i);
		i++;
	}
	return (-1);
}

void	hx_present(t_wsrv *s, t_handle c, int slot, uint64_t *st)
{
	t_wmpresent	p;
	t_wwin		*w;
	t_rect		r;

	w = &s->t.w[slot];
	if (w->content.px == NULL)
		return ;
	r = rect_make((int32_t)hg_below(st, (uint32_t)w->content.w),
			(int32_t)hg_below(st, (uint32_t)w->content.h),
			1 + (int32_t)hg_below(st, 80), 1 + (int32_t)hg_below(st, 60));
	gfx_fill(&w->content, r, 0xff000000 | (uint32_t)hg_next(st));
	memset(&p, 0, sizeof(p));
	ws_hdr(&p.h, WMC_PRESENT, sizeof(p), w->id);
	p.nrects = 1;
	p.rects[0] = r;
	if (r.x + r.w > w->content.w || r.y + r.h > w->content.h)
		p.nrects = 0;
	hr_send(s, c, &p);
}

void	hx_input(t_wsrv *s, uint64_t *st, uint32_t k)
{
	t_point	a;
	t_point	b;

	a = (t_point){(int32_t)hg_below(st, 320), (int32_t)hg_below(st, 240)};
	b = (t_point){(int32_t)hg_below(st, 320), (int32_t)hg_below(st, 240)};
	if (k == 0)
		hs_point(s, a.x, a.y);
	else if (k == 1)
		hs_click(s, a.x, a.y);
	else if (k == 2)
		hs_drag(s, a, b);
	else if (k == 3)
		hs_key(s, VK_TAB, INPM_ALT);
	else
		hs_key(s, VK_MENU, 0);
}
