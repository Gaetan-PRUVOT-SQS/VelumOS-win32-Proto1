#include "velum/err.h"
#include "velum/libk.h"
#include "ws_srv.h"

static uint64_t	section_bytes(t_rect cl, int32_t *dim)
{
	uint64_t	bytes;

	dim[0] = wg_clamp(cl.w, 1, WS_DIM_MAX);
	dim[1] = wg_clamp(cl.h, 1, WS_DIM_MAX);
	bytes = (uint64_t)dim[0] * (uint64_t)dim[1] * 4;
	return ((bytes + WS_PAGE - 1) & ~(WS_PAGE - 1));
}

void	ws_section_release(t_wsrv *s, int slot)
{
	t_wwin	*w;

	w = &s->t.w[slot];
	if (w->section == 0)
		return ;
	ws_sys_unmap(w->content.px, w->sec_bytes);
	ws_sys_close(w->section);
	s->c[w->owner].sec_bytes -= w->sec_bytes;
	s->sec_total -= w->sec_bytes;
	w->section = 0;
	w->sec_bytes = 0;
	memset(&w->content, 0, sizeof(w->content));
}

static void	install(t_wsrv *s, int slot, t_handle h, uint32_t *px)
{
	t_wwin	*w;
	int32_t	dim[2];

	w = &s->t.w[slot];
	w->sec_bytes = section_bytes(wh_client(&s->t, slot), dim);
	w->section = h;
	w->content.px = px;
	w->content.w = dim[0];
	w->content.h = dim[1];
	w->content.stride = dim[0];
	w->content.clip = rect_make(0, 0, dim[0], dim[1]);
	s->c[w->owner].sec_bytes += w->sec_bytes;
	s->sec_total += w->sec_bytes;
}

int	ws_section_attach(t_wsrv *s, int slot)
{
	t_wwin		*w;
	int32_t		dim[2];
	uint64_t	bytes;
	int64_t		h;
	void		*px;

	w = &s->t.w[slot];
	bytes = section_bytes(wh_client(&s->t, slot), dim);
	if (s->c[w->owner].sec_bytes - w->sec_bytes + bytes > WM_SECTION_MAX
		|| s->sec_total - w->sec_bytes + bytes > WS_SECTION_TOTAL)
		return (E_NOSPC);
	h = ws_sys_section(bytes);
	if (h <= 0)
		return (E_NOMEM);
	px = ws_sys_map((t_handle)h, bytes);
	if (px == NULL)
	{
		ws_sys_close((t_handle)h);
		return (E_NOMEM);
	}
	ws_section_release(s, slot);
	install(s, slot, (t_handle)h, px);
	return (0);
}

int	ws_send_section(t_wsrv *s, int slot, uint32_t type, uint32_t seq)
{
	t_wmcreated	m;
	t_wwin		*w;

	w = &s->t.w[slot];
	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, type, sizeof(m), w->id);
	m.h.seq = seq;
	m.stride = (uint32_t)w->content.stride;
	m.width = (uint32_t)w->content.w;
	m.height = (uint32_t)w->content.h;
	return (ws_post_handle(s, w->owner, &m, w->section));
}
