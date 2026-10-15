#include "velum/err.h"
#include "velum/libk.h"
#include "wmc_int.h"

static uint32_t	fill_rects(t_wmpresent *m, const t_wmwin *w,
	const t_rect *rects, uint32_t n)
{
	t_rect		bound;
	t_rect		r;
	uint32_t	i;

	bound = rect_make(0, 0, w->surface.w, w->surface.h);
	m->nrects = 0;
	i = 0;
	while (i < n && m->nrects < WM_RECTS_MAX)
	{
		r = rect_intersect(rects[i], bound);
		if (!rect_empty(r))
			m->rects[m->nrects++] = r;
		i++;
	}
	return (i);
}

int	wmc_present(t_wmwin *w, const t_rect *rects, uint32_t n)
{
	t_wmpresent	m;
	uint32_t	done;
	int			r;

	if (w == NULL || (n > 0 && rects == NULL))
		return (E_INVAL);
	done = 0;
	r = 0;
	while (r >= 0 && (done < n || (n == 0 && done == 0)))
	{
		memset(&m, 0, sizeof(m));
		wmc_hdr(&m.h, WMC_PRESENT, sizeof(m), w->id);
		if (n > 0)
			done += fill_rects(&m, w, rects + done, n - done);
		else
			done = 1;
		if (n == 0 || m.nrects > 0)
			r = wmc_send(&m);
	}
	if (r < 0)
		return (r);
	return (0);
}

int	wmc_next(void *buf, uint32_t size, uint64_t timeout_ns)
{
	t_wmcmsg	*m;
	int			r;

	m = &g_wmc.rx;
	if (buf == NULL)
		return (E_INVAL);
	if (!wmc_stash_pop(m))
	{
		r = wmc_recv(m, timeout_ns);
		if (r < 0)
			return (r);
	}
	wmc_on_message(m);
	if (size < m->len)
		return (E_RANGE);
	memcpy(buf, m->buf, m->len);
	return ((int)m->len);
}
