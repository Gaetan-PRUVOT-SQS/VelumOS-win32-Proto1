#include "velum/err.h"
#include "velum/libk.h"
#include "velum/random.h"
#include "velum/timer.h"
#include "net_int.h"

#define DHCP_MASK_DEFAULT 0xffffff00u

typedef struct s_dhcpctx
{
	int			id;
	t_sock		*sock;
	uint32_t	xid;
	uint8_t		mac[6];
}	t_dhcpctx;

static bool	sock_ready(void *arg)
{
	return (((t_sock *)arg)->count > 0);
}

static int	dhcp_send(const t_dhcpctx *c, uint8_t type, uint32_t req_ip,
		uint32_t server)
{
	uint8_t	buf[DHCP_MAX];
	size_t	n;
	int		rc;

	n = dhcp_build(buf, sizeof(buf), type, c->mac, c->xid, req_ip, server);
	if (n == 0)
		return (E_INVAL);
	mutex_lock(&g_net.lock);
	rc = net_udp_send_l(DHCP_CLIENT_PORT, NET_IP_BROADCAST, DHCP_SERVER_PORT,
			buf, n);
	mutex_unlock(&g_net.lock);
	return (rc);
}

static int	dhcp_take(const t_dhcpctx *c, uint8_t want, t_dhcpinfo *out)
{
	t_dgram	d;
	int		rc;

	mutex_lock(&g_net.lock);
	rc = sock_take_l(c->sock, &d);
	mutex_unlock(&g_net.lock);
	if (rc < 0)
		return (rc);
	if (dhcp_parse(d.data, d.len, c->xid, c->mac, out) < 0)
		return (E_AGAIN);
	if (out->type == DHCP_NAK)
		return (E_PROTO);
	if (out->type != want)
		return (E_AGAIN);
	return (0);
}

static int	dhcp_expect(const t_dhcpctx *c, uint8_t want, t_dhcpinfo *out)
{
	uint64_t	deadline;
	uint64_t	now;
	int			rc;

	deadline = time_now_ns() + NET_DHCP_STEP_NS;
	rc = E_TIMEOUT;
	now = time_now_ns();
	while (now < deadline)
	{
		if (net_wait(sock_ready, c->sock, deadline - now) < 0)
			break ;
		rc = dhcp_take(c, want, out);
		if (rc != E_AGAIN)
			return (rc);
		now = time_now_ns();
	}
	return (E_TIMEOUT);
}

static int	dhcp_exchange(const t_dhcpctx *c, t_dhcpinfo *lease)
{
	t_dhcpinfo	offer;
	int			rc;

	rc = dhcp_send(c, DHCP_DISCOVER, 0, 0);
	if (rc < 0)
		return (rc);
	rc = dhcp_expect(c, DHCP_OFFER, &offer);
	if (rc < 0 || offer.yiaddr == 0 || offer.yiaddr == NET_IP_BROADCAST)
		return (rc < 0 ? rc : E_PROTO);
	rc = dhcp_send(c, DHCP_REQUEST, offer.yiaddr, offer.server);
	if (rc < 0)
		return (rc);
	rc = dhcp_expect(c, DHCP_ACK, lease);
	if (rc < 0 || lease->yiaddr != offer.yiaddr)
		return (rc < 0 ? rc : E_PROTO);
	return (0);
}

static void	dhcp_apply(const t_dhcpinfo *l)
{
	mutex_lock(&g_net.lock);
	g_net.ip = l->yiaddr;
	g_net.mask = l->mask ? l->mask : DHCP_MASK_DEFAULT;
	g_net.gw = l->router;
	g_net.dns = l->dns;
	g_net.lease_s = l->lease;
	g_net.etat = NET_ETAT_PRET;
	mutex_unlock(&g_net.lock);
}

static int	dhcp_open(t_dhcpctx *c)
{
	int	rc;

	mutex_lock(&g_net.lock);
	rc = sock_alloc_l(0, NULL);
	if (rc >= 0)
	{
		c->id = rc;
		c->sock = sock_get_l((uint32_t)rc, 0);
		rc = sock_bind_l(c->sock, DHCP_CLIENT_PORT, true);
		if (rc < 0)
			sock_free_l(c->sock);
	}
	memcpy(c->mac, g_net.mac, 6);
	mutex_unlock(&g_net.lock);
	c->xid = (uint32_t)krandom_u64();
	return (rc < 0 ? rc : 0);
}

int	net_dhcp_run(void)
{
	t_dhcpctx	c;
	t_dhcpinfo	lease;
	int			rc;
	int			tries;

	if (!g_net.ready || !g_net.nic)
		return (E_NODEV);
	rc = dhcp_open(&c);
	if (rc < 0)
		return (rc);
	tries = 0;
	rc = E_TIMEOUT;
	while (tries < NET_DHCP_TRIES && rc == E_TIMEOUT)
	{
		rc = dhcp_exchange(&c, &lease);
		tries++;
	}
	if (rc == 0)
		dhcp_apply(&lease);
	mutex_lock(&g_net.lock);
	sock_free_l(c.sock);
	mutex_unlock(&g_net.lock);
	return (rc);
}
