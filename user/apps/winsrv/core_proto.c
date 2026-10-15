#include "velum/err.h"
#include "velum/libk.h"
#include "ws_core.h"

uint32_t	wsp_size_of(uint32_t type)
{
	static const uint32_t	sizes[] = {0, sizeof(t_wmhello),
		sizeof(t_wmcreate), sizeof(t_wmhdr), sizeof(t_wmtitle),
		sizeof(t_wmrect), sizeof(t_wmarg), sizeof(t_wmpresent),
		sizeof(t_wmarg), sizeof(t_wmhdr), sizeof(t_wmrect),
		sizeof(t_wmarg), sizeof(t_wmarg), sizeof(t_wmarg)};

	if (type >= sizeof(sizes) / sizeof(sizes[0]))
		return (0);
	return (sizes[type]);
}

static bool	present_ok(const t_wmpresent *p)
{
	uint32_t	i;
	t_rect		r;

	if (p->nrects > WM_RECTS_MAX)
		return (false);
	i = 0;
	while (i < p->nrects)
	{
		r = p->rects[i];
		if (r.x < 0 || r.y < 0 || r.x > WS_DIM_MAX || r.y > WS_DIM_MAX
			|| r.w < 0 || r.h < 0 || r.w > WS_DIM_MAX || r.h > WS_DIM_MAX)
			return (false);
		i++;
	}
	return (true);
}

static uint32_t	arg_limit(uint32_t type)
{
	if (type == WMC_SET_STATE)
		return (WSTATE_HIDDEN);
	if (type == WMC_SET_CURSOR)
		return (CUR_HAND);
	if (type == WMC_SET_ICON)
		return (ICON_IDS - 1);
	return (1);
}

static bool	body_ok(const void *buf, uint32_t type)
{
	const t_wmcreate	*c;
	const t_wmarg		*a;

	c = buf;
	a = buf;
	if (type == WMC_HELLO)
		return (((const t_wmhello *)buf)->version == WM_VERSION);
	if (type == WMC_CREATE)
		return (wsp_rect_ok(c->rect) && (c->style & ~WS_STYLE_KNOWN) == 0
			&& c->state <= WSTATE_HIDDEN
			&& wsp_title_ok(c->title, WM_TITLE_MAX));
	if (type == WMC_SET_TITLE)
		return (wsp_title_ok(((const t_wmtitle *)buf)->title, WM_TITLE_MAX));
	if (type == WMC_SET_RECT || type == WMC_SET_WORKAREA)
		return (wsp_rect_ok(((const t_wmrect *)buf)->rect));
	if (type == WMC_PRESENT)
		return (present_ok(buf));
	return (wsp_size_of(type) != sizeof(t_wmarg)
		|| (a->value <= arg_limit(type) && a->reserved == 0));
}

int	wsp_validate(const void *buf, uint32_t len, uint32_t nhandles)
{
	t_wmhdr	h;
	bool	global;

	if (buf == NULL || nhandles != 0 || len < sizeof(t_wmhdr)
		|| len > IPC_MSG_MAX)
		return (E_PROTO);
	memcpy(&h, buf, sizeof(h));
	if (h.magic != WM_MAGIC || h.size != len || h.status != 0
		|| wsp_size_of(h.type) != len)
		return (E_PROTO);
	global = (h.type == WMC_HELLO || h.type == WMC_CREATE
			|| h.type == WMC_SET_WORKAREA || h.type == WMC_SUBSCRIBE);
	if (global != (h.window == 0))
		return (E_PROTO);
	if (!body_ok(buf, h.type))
		return (E_PROTO);
	return (0);
}
