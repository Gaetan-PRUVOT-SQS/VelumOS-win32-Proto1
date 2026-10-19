#include "velum/err.h"
#include "velum/libk.h"
#include "velum/notif.h"
#include "velum/vobj.h"
#include "velum/vtime.h"

#define NOTIF_REPLY_NS 30000000ull
#define NOTIF_CACHE_NS 250000000ll

static t_handle	g_chan;
static uint32_t	g_seq;
static uint32_t	g_cached;
static int64_t	g_cached_at;

void	notif_client_close(void)
{
	if (g_chan != 0)
		v_close(g_chan);
	g_chan = 0;
	g_cached_at = 0;
}

static int	ensure_channel(void)
{
	int64_t	h;

	if (g_chan != 0)
		return (0);
	h = v_port_connect(NOTIF_PORT_NAME, 0);
	if (h < 0)
		return ((int)h);
	g_chan = (t_handle)h;
	return (0);
}

static int	send_msg(t_notifmsg *m)
{
	t_chansend	s;
	int			r;

	r = ensure_channel();
	if (r < 0)
		return (r);
	g_seq++;
	m->seq = g_seq;
	memset(&s, 0, sizeof(s));
	s.msg = (uint64_t)(uintptr_t)m;
	s.len = sizeof(*m);
	r = v_chan_send(g_chan, &s);
	if (r < 0)
		notif_client_close();
	return (r);
}

static int	recv_msg(t_notifmsg *m)
{
	t_chanrecv	r;
	int			ret;

	memset(&r, 0, sizeof(r));
	r.buf = (uint64_t)(uintptr_t)m;
	r.buf_len = sizeof(*m);
	r.timeout_ns = NOTIF_REPLY_NS;
	ret = v_chan_recv(g_chan, &r);
	if (ret < 0)
		return (ret);
	if (r.out_len != sizeof(*m) || m->magic != NOTIF_MAGIC)
		return (E_INVAL);
	return (0);
}

static int	call(t_notifmsg *m)
{
	int	r;

	r = send_msg(m);
	if (r < 0)
		return (r);
	r = recv_msg(m);
	if (r < 0)
		return (r);
	return (m->status);
}

int	notif_post(const char *app, const char *title, const char *text,
	uint32_t icon)
{
	t_notifmsg	m;
	int			r;

	r = notif_msg_post(&m, app, title, text, icon);
	if (r < 0)
		return (r);
	r = call(&m);
	if (r < 0)
		return (r);
	return ((int)m.id);
}

int	notif_client_count(uint32_t *out)
{
	t_notifmsg	m;
	int64_t		now;
	int			r;

	if (out == NULL)
		return (E_INVAL);
	now = v_time_mono();
	if (g_cached_at != 0 && now - g_cached_at < NOTIF_CACHE_NS)
	{
		*out = g_cached;
		return (0);
	}
	notif_msg_init(&m, NOTIFC_COUNT);
	r = call(&m);
	if (r < 0)
		return (r);
	g_cached = m.count;
	g_cached_at = now;
	*out = g_cached;
	return (0);
}

static int	simple(uint32_t type, uint32_t id)
{
	t_notifmsg	m;

	notif_msg_init(&m, type);
	m.id = id;
	g_cached_at = 0;
	return (call(&m));
}

int	notif_client_toggle(void)
{
	return (simple(NOTIFC_TOGGLE, 0));
}

int	notif_client_dismiss(uint32_t id)
{
	return (simple(NOTIFC_DISMISS, id));
}

int	notif_client_clear(void)
{
	return (simple(NOTIFC_CLEAR, 0));
}
