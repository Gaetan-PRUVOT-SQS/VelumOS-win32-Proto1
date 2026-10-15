#include "velum/err.h"
#include "velum/libk.h"
#include "wmc_int.h"

static bool	created_ok(const t_wmcmsg *m)
{
	const t_wmcreated	*c;

	c = (const t_wmcreated *)m->buf;
	return (m->handle != 0 && c->width != 0 && c->height != 0
		&& c->width <= c->stride && c->stride <= WMC_DIM_MAX
		&& c->height <= WMC_DIM_MAX);
}

static void	set_surface(t_surface *s, const t_wmcreated *c, void *px)
{
	s->px = px;
	s->w = (int32_t)c->width;
	s->h = (int32_t)c->height;
	s->stride = (int32_t)c->stride;
	s->clip = rect_make(0, 0, s->w, s->h);
}

int	wmc_attach(t_wmwin *w, const t_wmcmsg *m)
{
	const t_wmcreated	*c;
	uint64_t			len;
	void				*px;

	c = (const t_wmcreated *)m->buf;
	if (!created_ok(m))
	{
		wmsys_close(m->handle);
		return (E_PROTO);
	}
	len = ((uint64_t)c->stride * c->height * 4 + WMC_PAGE - 1)
		& ~(WMC_PAGE - 1);
	px = wmsys_map(m->handle, len);
	if (px == NULL)
	{
		wmsys_close(m->handle);
		return (E_NOMEM);
	}
	wmc_detach(w);
	w->section = m->handle;
	set_surface(&w->surface, c, px);
	return (0);
}

static int	bind(t_wmwin *w, uint32_t style)
{
	int	r;

	memset(w, 0, sizeof(*w));
	w->id = ((const t_wmhdr *)g_wmc.rx.buf)->window;
	w->chan = g_wmc.chan;
	w->style = style;
	r = wmc_attach(w, &g_wmc.rx);
	if (r < 0)
		wmc_simple(WMC_DESTROY, w->id, 0);
	return (r);
}

int	wmc_create(t_wmwin *w, const t_wmcreate *rq)
{
	t_wmcreate	m;
	int			k;
	int			r;

	if (w == NULL || rq == NULL)
		return (E_INVAL);
	k = wmc_slot(0);
	if (k < 0)
		return (E_NOSPC);
	m = *rq;
	wmc_hdr(&m.h, WMC_CREATE, sizeof(m), 0);
	wmc_title_copy(m.title, rq->title);
	r = wmc_send(&m);
	if (r >= 0)
		r = wmc_wait((uint32_t)r, WMS_CREATED, &g_wmc.rx);
	if (r < 0)
		return (r);
	r = bind(w, rq->style);
	if (r == 0)
		g_wmc.wins[k] = w;
	return (r);
}
