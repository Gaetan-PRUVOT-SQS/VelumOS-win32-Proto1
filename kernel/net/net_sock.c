#include "velum/err.h"
#include "velum/heap.h"
#include "velum/libk.h"
#include "velum/random.h"
#include "net_int.h"

#define SOCK_INDEX_BITS 8

static bool	port_taken(uint16_t port)
{
	uint32_t	i;

	i = 0;
	while (i < NET_SOCKS)
	{
		if (g_net.socks[i].used && g_net.socks[i].bound
			&& g_net.socks[i].port == port)
			return (true);
		i++;
	}
	return (false);
}

static void	sock_reclaim_l(t_pidalive alive)
{
	uint32_t	i;

	if (!alive)
		return ;
	i = 0;
	while (i < NET_SOCKS)
	{
		if (g_net.socks[i].used && g_net.socks[i].owner != 0
			&& !alive(g_net.socks[i].owner))
			sock_free_l(&g_net.socks[i]);
		i++;
	}
}

static int	sock_find_free(void)
{
	int	i;

	i = 0;
	while (i < NET_SOCKS)
	{
		if (!g_net.socks[i].used)
			return (i);
		i++;
	}
	return (-1);
}

uint32_t	sock_id(const t_sock *s, uint32_t index)
{
	return ((s->gen << SOCK_INDEX_BITS) | index);
}

int	sock_alloc_l(uint32_t owner, t_pidalive alive)
{
	int		i;
	t_sock	*s;

	i = sock_find_free();
	if (i < 0)
	{
		sock_reclaim_l(alive);
		i = sock_find_free();
	}
	if (i < 0)
		return (E_NFILE);
	s = &g_net.socks[i];
	s->queue = kmalloc(sizeof(t_dgram) * NET_SOCK_QUEUE);
	if (!s->queue)
		return (E_NOMEM);
	s->used = true;
	s->bound = false;
	s->owner = owner;
	s->port = 0;
	s->head = 0;
	s->count = 0;
	s->gen++;
	return ((int)sock_id(s, (uint32_t)i));
}

t_sock	*sock_get_l(uint32_t id, uint32_t owner)
{
	t_sock	*s;

	if ((id & ((1u << SOCK_INDEX_BITS) - 1)) >= NET_SOCKS)
		return (NULL);
	s = &g_net.socks[id & ((1u << SOCK_INDEX_BITS) - 1)];
	if (!s->used || s->gen != (id >> SOCK_INDEX_BITS) || s->owner != owner)
		return (NULL);
	return (s);
}

void	sock_free_l(t_sock *s)
{
	kfree(s->queue);
	s->queue = NULL;
	s->used = false;
	s->bound = false;
	s->count = 0;
}

static int	ephemeral_port(void)
{
	int			tries;
	uint16_t	port;

	tries = 0;
	while (tries < 64)
	{
		port = (uint16_t)(NET_PORT_EPHEMERAL_MIN
				+ krandom_below(NET_PORT_EPHEMERAL_SPAN));
		if (!port_taken(port))
			return (port);
		tries++;
	}
	return (E_BUSY);
}

int	sock_bind_l(t_sock *s, uint16_t port, bool privileged)
{
	int	rc;

	if (s->bound)
		return (E_INVAL);
	if (port == 0)
	{
		rc = ephemeral_port();
		if (rc < 0)
			return (rc);
		port = (uint16_t)rc;
	}
	else if (port < NET_PORT_PRIVILEGED && !privileged)
		return (E_PERM);
	if (port_taken(port))
		return (E_BUSY);
	s->port = port;
	s->bound = true;
	return (port);
}

bool	sock_deliver_l(uint16_t port, uint32_t ip, uint16_t from,
		const uint8_t *data, size_t len)
{
	uint32_t	i;
	t_sock		*s;
	t_dgram		*d;

	i = 0;
	while (i < NET_SOCKS)
	{
		s = &g_net.socks[i++];
		if (!s->used || !s->bound || s->port != port)
			continue ;
		if (s->count >= NET_SOCK_QUEUE || len > NET_UDP_MAX)
			return (false);
		d = &s->queue[(s->head + s->count) % NET_SOCK_QUEUE];
		d->ip = ip;
		d->port = from;
		d->len = (uint16_t)len;
		memcpy(d->data, data, len);
		s->count++;
		return (true);
	}
	return (false);
}

int	sock_take_l(t_sock *s, t_dgram *out)
{
	if (s->count == 0)
		return (E_AGAIN);
	memcpy(out, &s->queue[s->head], sizeof(*out));
	s->head = (uint8_t)((s->head + 1) % NET_SOCK_QUEUE);
	s->count--;
	return (0);
}
