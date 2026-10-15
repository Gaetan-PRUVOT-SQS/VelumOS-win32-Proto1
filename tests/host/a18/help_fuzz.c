#include <string.h>
#include "help.h"

static uint32_t	some_id(const t_wsrv *s, uint64_t *st)
{
	uint32_t	slot;

	if (hg_below(st, 8) == 0)
		return ((uint32_t)hg_next(st));
	slot = hg_below(st, WS_WIN_MAX);
	if (s->t.w[slot].id == 0)
		return (slot + 1);
	return (s->t.w[slot].id);
}

static void	valid_body(uint8_t *buf, uint32_t type, uint64_t *st)
{
	t_wmcreate	*c;
	uint32_t	i;

	c = (t_wmcreate *)buf;
	if (type == WMC_HELLO)
		((t_wmhello *)buf)->version = WM_VERSION;
	if (type == WMC_CREATE || type == WMC_SET_RECT
		|| type == WMC_SET_WORKAREA)
		c->rect = hx_rect(st);
	if (type == WMC_CREATE)
		c->style = hg_below(st, WS_STYLE_KNOWN + 1);
	if (type == WMC_CREATE || type == WMC_SET_TITLE)
		memcpy(((t_wmtitle *)buf)->title, "Fen\xc3\xaatre", 9);
	if (type == WMC_PRESENT)
	{
		((t_wmpresent *)buf)->nrects = hg_below(st, WM_RECTS_MAX + 1);
		i = 0;
		while (i < WM_RECTS_MAX)
			((t_wmpresent *)buf)->rects[i++] = rect_make(0, 0,
					(int32_t)hg_below(st, 300), (int32_t)hg_below(st, 300));
	}
	if (wsp_size_of(type) == sizeof(t_wmarg))
		((t_wmarg *)buf)->value = hg_below(st, 4);
}

static void	mutate(uint8_t *buf, uint32_t *len, uint64_t *st)
{
	uint32_t	n;

	n = hg_below(st, 4);
	while (n-- > 0 && *len > 0)
		buf[hg_below(st, *len)] ^= (uint8_t)(1u << hg_below(st, 8));
	if (hg_below(st, 10) == 0)
		*len = hg_below(st, 400);
}

uint32_t	hf_message(const t_wsrv *s, uint8_t *buf, uint64_t *st)
{
	uint32_t	type;
	uint32_t	len;
	t_wmhdr		h;

	memset(buf, 0, 512);
	type = 1 + hg_below(st, WMC_CAPTURE);
	len = wsp_size_of(type);
	if (hg_below(st, 10) < 3)
	{
		len = hg_below(st, 400);
		while (type < len)
			buf[type++ % 512] = (uint8_t)hg_next(st);
		return (len);
	}
	ws_hdr(&h, type, len, some_id(s, st));
	if (type == WMC_HELLO || type == WMC_CREATE || type == WMC_SUBSCRIBE
		|| type == WMC_SET_WORKAREA)
		h.window = 0;
	memcpy(buf, &h, sizeof(h));
	valid_body(buf, type, st);
	if (hg_below(st, 2))
		mutate(buf, &len, st);
	return (len);
}
