#include "velum/err.h"
#include "velum/libk.h"
#include "ws_srv.h"

static int	enqueue(t_wclient *c, const void *msg, uint32_t len, t_handle h)
{
	t_wout	*o;

	if (c->nout >= WS_OUT_MAX || len > sizeof(c->out[0].msg))
		return (E_AGAIN);
	o = &c->out[(c->head + c->nout) % WS_OUT_MAX];
	memcpy(o->msg, msg, len);
	o->len = len;
	o->handle = h;
	c->nout++;
	return (0);
}

static t_wclient	*alive(t_wsrv *s, int ci)
{
	if (ci < 0 || ci >= WS_CLIENT_MAX)
		return (NULL);
	if ((s->c[ci].flags & (WCF_USED | WCF_DEAD)) != WCF_USED)
		return (NULL);
	return (&s->c[ci]);
}

void	ws_post(t_wsrv *s, int ci, const void *msg, uint32_t flags)
{
	t_wclient	*c;
	t_wmhdr		h;
	int			r;

	c = alive(s, ci);
	if (c == NULL)
		return ;
	memcpy(&h, msg, sizeof(h));
	if (c->nout == 0)
	{
		r = ws_sys_send(c->chan, msg, h.size, 0);
		if (r >= 0)
			return ;
		if (r != E_AGAIN)
		{
			c->flags |= WCF_DEAD;
			return ;
		}
	}
	if ((flags & WSO_DROP) || enqueue(c, msg, h.size, 0) < 0)
		c->dropped++;
}

int	ws_post_handle(t_wsrv *s, int ci, const void *msg, t_handle h)
{
	t_wclient	*c;
	t_wmhdr		hd;
	int			r;

	c = alive(s, ci);
	if (c == NULL)
		return (E_PIPE);
	memcpy(&hd, msg, sizeof(hd));
	if (c->nout == 0)
	{
		r = ws_sys_send(c->chan, msg, hd.size, h);
		if (r >= 0)
			return (0);
		if (r != E_AGAIN)
		{
			c->flags |= WCF_DEAD;
			return (r);
		}
	}
	if (enqueue(c, msg, hd.size, h) < 0)
	{
		c->flags |= WCF_DEAD;
		return (E_AGAIN);
	}
	return (0);
}

void	ws_flush(t_wsrv *s, int ci)
{
	t_wclient	*c;
	t_wout		*o;
	int			r;

	c = alive(s, ci);
	while (c != NULL && c->nout > 0)
	{
		o = &c->out[c->head];
		r = ws_sys_send(c->chan, o->msg, o->len, o->handle);
		if (r == E_AGAIN)
			return ;
		if (r < 0 && o->handle == 0)
		{
			c->flags |= WCF_DEAD;
			return ;
		}
		c->head = (c->head + 1) % WS_OUT_MAX;
		c->nout--;
	}
}
