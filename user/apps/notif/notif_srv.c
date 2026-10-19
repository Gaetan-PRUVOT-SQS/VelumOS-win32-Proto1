#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vobj.h"
#include "notif_app.h"

int	notifsrv_start(t_notifapp *app)
{
	int64_t	h;

	h = v_port_listen(NOTIF_PORT_NAME, 0);
	if (h <= 0)
		return (h < 0 ? (int)h : E_BADF);
	app->listener = (t_handle)h;
	return (0);
}

static int	reply(t_handle chan, const t_notifmsg *m)
{
	t_chansend	s;

	memset(&s, 0, sizeof(s));
	s.msg = (uint64_t)(uintptr_t)m;
	s.len = sizeof(*m);
	return (v_chan_send(chan, &s));
}

static void	accept_new(t_notifapp *app)
{
	int64_t		h;
	uint32_t	i;

	h = v_port_accept(app->listener, 0);
	while (h > 0)
	{
		i = 0;
		while (i < NA_CLIENTS_MAX && app->clients[i] != 0)
			i++;
		if (i == NA_CLIENTS_MAX)
			v_close((t_handle)h);
		else
			app->clients[i] = (t_handle)h;
		h = v_port_accept(app->listener, 0);
	}
}

static void	answer(t_notifapp *app, uint32_t i, const t_chanrecv *r,
	const t_notifmsg *raw)
{
	t_notifmsg	m;
	uint32_t	seq;

	if (notif_msg_decode(raw, (uint32_t)r->out_len, &m) < 0)
	{
		memset(&m, 0, sizeof(m));
		notif_msg_init(&m, NOTIFS_ERROR);
		m.status = E_INVAL;
		reply(app->clients[i], &m);
		return ;
	}
	seq = m.seq;
	notifsrv_handle(app, &m);
	m.seq = seq;
	if (reply(app->clients[i], &m) == E_PIPE)
	{
		v_close(app->clients[i]);
		app->clients[i] = 0;
	}
}

static void	serve_client(t_notifapp *app, uint32_t i)
{
	t_notifmsg	m;
	t_chanrecv	r;
	int			ret;

	memset(&r, 0, sizeof(r));
	r.buf = (uint64_t)(uintptr_t)&m;
	r.buf_len = sizeof(m);
	ret = v_chan_recv(app->clients[i], &r);
	if (ret == E_TIMEOUT || ret == E_AGAIN)
		return ;
	if (ret < 0)
	{
		v_close(app->clients[i]);
		app->clients[i] = 0;
		return ;
	}
	answer(app, i, &r, &m);
}

void	notifsrv_poll(t_notifapp *app)
{
	uint32_t	i;

	accept_new(app);
	i = 0;
	while (i < NA_CLIENTS_MAX)
	{
		if (app->clients[i] != 0)
			serve_client(app, i);
		i++;
	}
}

void	notifsrv_stop(t_notifapp *app)
{
	uint32_t	i;

	i = 0;
	while (i < NA_CLIENTS_MAX)
	{
		if (app->clients[i] != 0)
			v_close(app->clients[i]);
		app->clients[i] = 0;
		i++;
	}
	if (app->listener != 0)
		v_close(app->listener);
	app->listener = 0;
}
