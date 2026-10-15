#include "velum/err.h"
#include "velum/libk.h"
#include "wmc_int.h"

t_wmcstate	g_wmc;

void	wmc_hdr(t_wmhdr *h, uint32_t type, uint32_t size, uint32_t window)
{
	h->magic = WM_MAGIC;
	h->type = type;
	h->size = size;
	h->window = window;
	h->seq = 0;
	h->status = 0;
}

int	wmc_send(void *msg)
{
	t_wmhdr	*h;
	int		r;

	if (g_wmc.chan == 0)
		return (E_BADF);
	if (g_wmc.lost)
		return (E_PIPE);
	h = msg;
	g_wmc.seq++;
	if (g_wmc.seq == 0 || g_wmc.seq > 0x7fffffff)
		g_wmc.seq = 1;
	h->seq = g_wmc.seq;
	r = wmsys_send(g_wmc.chan, msg, h->size);
	if (r == E_PIPE)
		g_wmc.lost = true;
	if (r < 0)
		return (r);
	return ((int)h->seq);
}

void	wmc_disconnect(void)
{
	t_wmcmsg	m;
	int			k;

	k = 0;
	while (k < WM_WIN_PER_CLIENT)
	{
		if (g_wmc.wins[k] != NULL)
			wmc_detach(g_wmc.wins[k]);
		k++;
	}
	while (wmc_stash_pop(&m))
		if (m.handle != 0)
			wmsys_close(m.handle);
	if (g_wmc.chan != 0)
		wmsys_close(g_wmc.chan);
	memset(&g_wmc, 0, sizeof(g_wmc));
}

static int	hello(void)
{
	t_wmhello	rq;
	int			r;

	memset(&rq, 0, sizeof(rq));
	wmc_hdr(&rq.h, WMC_HELLO, sizeof(rq), 0);
	rq.version = WM_VERSION;
	r = wmc_send(&rq);
	if (r >= 0)
		r = wmc_wait((uint32_t)r, WMS_HELLO_OK, &g_wmc.rx);
	return (r);
}

int	wmc_connect(t_wmhello *info)
{
	int64_t		h;
	int			r;

	if (g_wmc.chan != 0)
		return (E_BUSY);
	h = wmsys_connect(WM_PORT_NAME);
	if (h == 0)
		return (E_IO);
	if (h < 0)
		return ((int)h);
	memset(&g_wmc, 0, sizeof(g_wmc));
	g_wmc.chan = (t_handle)h;
	r = hello();
	if (r < 0)
	{
		wmc_disconnect();
		return (r);
	}
	memcpy(&g_wmc.info, g_wmc.rx.buf, sizeof(g_wmc.info));
	if (info != NULL)
		*info = g_wmc.info;
	return (0);
}
