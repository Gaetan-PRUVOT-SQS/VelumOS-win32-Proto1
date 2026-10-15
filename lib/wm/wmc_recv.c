#include "velum/err.h"
#include "velum/libk.h"
#include "wmc_int.h"

static uint32_t	expected(uint32_t type)
{
	static const uint32_t	sizes[] = {sizeof(t_wmhello),
		sizeof(t_wmcreated), sizeof(t_wmkey), sizeof(t_wmmouse),
		sizeof(t_wmhdr), sizeof(t_wmarg), sizeof(t_wmcreated),
		sizeof(t_wmhdr), sizeof(t_wmarg), sizeof(t_wminfo),
		sizeof(t_wminfo), sizeof(t_wminfo)};

	if (type == WMS_ERROR)
		return (sizeof(t_wmhdr));
	if (type < WMS_HELLO_OK || type > WMS_WIN_UPD)
		return (0);
	return (sizes[type - WMS_HELLO_OK]);
}

bool	wmc_valid(const t_wmcmsg *m)
{
	t_wmhdr	h;
	bool	with_handle;

	if (m->len < sizeof(t_wmhdr) || m->len > sizeof(m->buf))
		return (false);
	memcpy(&h, m->buf, sizeof(h));
	if (h.magic != WM_MAGIC || h.size != m->len
		|| expected(h.type) != m->len)
		return (false);
	with_handle = (h.type == WMS_CREATED || h.type == WMS_RESIZED);
	return (with_handle == (m->handle != 0));
}

int	wmc_recv(t_wmcmsg *m, uint64_t timeout_ns)
{
	int	r;

	if (g_wmc.chan == 0)
		return (E_BADF);
	if (g_wmc.lost)
		return (E_PIPE);
	r = wmsys_recv(g_wmc.chan, m, timeout_ns);
	if (r == E_PIPE)
		g_wmc.lost = true;
	if (r < 0)
		return (r);
	if (!wmc_valid(m))
	{
		if (m->handle != 0)
			wmsys_close(m->handle);
		m->handle = 0;
		return (E_PROTO);
	}
	return ((int)m->len);
}

static int	reply_code(const t_wmhdr *h)
{
	if (h->type != WMS_ERROR)
		return (0);
	if (h->status < 0)
		return (h->status);
	return (E_PROTO);
}

int	wmc_wait(uint32_t seq, uint32_t type, t_wmcmsg *out)
{
	uint64_t	deadline;
	uint64_t	now;
	t_wmhdr		h;
	int			r;

	deadline = wmsys_now() + WM_REPLY_NS;
	now = wmsys_now();
	while (now < deadline)
	{
		r = wmc_recv(&g_wmc.rx, deadline - now);
		if (r < 0 && r != E_PROTO)
			return (r);
		memcpy(&h, g_wmc.rx.buf, sizeof(h));
		if (r >= 0 && h.seq == seq && (h.type == type || h.type == WMS_ERROR))
		{
			*out = g_wmc.rx;
			return (reply_code(&h));
		}
		if (r >= 0)
			wmc_stash_push(&g_wmc.rx);
		now = wmsys_now();
	}
	return (E_TIMEOUT);
}
