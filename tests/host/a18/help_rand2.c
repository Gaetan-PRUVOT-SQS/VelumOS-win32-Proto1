#include <string.h>
#include "help.h"

static void	set_rect(t_wsrv *s, t_handle c, uint32_t id, uint64_t *st)
{
	t_wmrect	m;

	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, WMC_SET_RECT, sizeof(m), id);
	m.rect = hx_rect(st);
	hr_send(s, c, &m);
}

static void	create(t_wsrv *s, t_handle c, uint64_t *st)
{
	static const uint32_t	styles[] = {WS_DEFAULT, WS_DEFAULT | WS_TOPMOST,
		WS_POPUP | WS_NOACTIVATE | WS_TOPMOST, WS_CAPTION | WS_SYSMENU,
		WS_DEFAULT | WS_TOOLWINDOW, WS_DEFAULT};

	hr_create(s, c, hx_rect(st), styles[hg_below(st, 6)]);
}

static void	act(t_wsrv *s, t_handle c, int slot, uint64_t *st)
{
	uint32_t	k;
	uint32_t	id;

	id = s->t.w[slot].id;
	k = hg_below(st, 6);
	if (k == 0)
		hr_destroy(s, c, id);
	else if (k == 1)
		hr_value(s, c, WMC_SET_STATE, (uint32_t [2]){id, hg_below(st, 4)});
	else if (k == 2)
		hr_value(s, c, WMC_SET_ICON, (uint32_t [2]){id,
			hg_below(st, ICON_IDS)});
	else if (k == 3)
		set_rect(s, c, id, st);
	else if (k == 4)
		hr_activate(s, c, id);
	else
		hx_present(s, c, slot, st);
}

void	hx_req(t_wsrv *s, const t_handle *cl, uint64_t *st)
{
	int	ci;
	int	slot;

	ci = (int)hg_below(st, 3);
	slot = hx_own(s, ci, st);
	if (slot < 0 || hg_below(st, 7) == 0)
		create(s, cl[ci], st);
	else
		act(s, cl[ci], slot, st);
}

int	hx_compare(t_wsrv *s, t_surface *ref)
{
	int32_t	x;
	int32_t	y;
	int		bad;

	wc_compose(&s->t, ref, s->cur, s->t.screen);
	bad = 0;
	y = 0;
	while (y < ref->h)
	{
		x = 0;
		while (x < ref->w)
		{
			bad += (ref->px[y * ref->stride + x] & 0xffffff)
				!= (s->fb.px[y * s->fb.stride + x] & 0xffffff);
			x++;
		}
		y++;
	}
	return (bad);
}
