#include "velum/err.h"
#include "velum/libk.h"
#include "velum/timer.h"
#include "net_int.h"

static void	echo_reply_l(const t_ethinfo *e, const t_ipinfo *ip,
		const t_icmpinfo *req)
{
	t_icmpinfo	rep;
	size_t		n;

	rep.type = ICMP_ECHO_REPLY;
	rep.id = req->id;
	rep.seq = req->seq;
	rep.data = req->data;
	rep.len = req->len;
	if (rep.len > NET_UDP_MAX - ICMP_HLEN)
		return ;
	n = icmp_build(g_net.l4, &rep);
	net_tx_l(e->src, ip->src, IP_PROTO_ICMP, g_net.l4, n);
}

void	icmp_input_l(const t_ethinfo *e, const t_ipinfo *ip)
{
	t_icmpinfo	m;

	if (icmp_parse(ip->data, ip->len, &m) < 0)
	{
		g_net.drops++;
		return ;
	}
	if (m.type == ICMP_ECHO_REQUEST && ip->dst == g_net.ip)
		echo_reply_l(e, ip, &m);
	else if (m.type == ICMP_ECHO_REPLY && g_net.ping_wait
		&& ip->src == g_net.ping_ip && m.seq == g_net.ping_seq)
	{
		g_net.ping_t1 = time_now_ns();
		g_net.ping_got = true;
	}
}

static bool	ping_done(void *arg)
{
	(void)arg;
	return (g_net.ping_got);
}

static int	ping_send_l(uint32_t ip)
{
	uint8_t		data[NET_PING_DATA];
	t_icmpinfo	m;
	uint8_t		mac[6];
	int			rc;
	uint32_t	i;

	i = 0;
	while (i < NET_PING_DATA)
	{
		data[i] = (uint8_t)i;
		i++;
	}
	rc = net_route_l(ip, mac);
	if (rc < 0)
		return (rc);
	m.type = ICMP_ECHO_REQUEST;
	m.id = 0x5656;
	m.seq = ++g_net.ping_seq;
	m.data = data;
	m.len = NET_PING_DATA;
	return (net_tx_l(mac, ip, IP_PROTO_ICMP, g_net.l4,
			icmp_build(g_net.l4, &m)));
}

static int	ping_begin(uint32_t ip, uint64_t *t0)
{
	int	rc;

	mutex_lock(&g_net.lock);
	rc = E_NODEV;
	if (g_net.etat == NET_ETAT_PRET)
	{
		rc = E_BUSY;
		if (!g_net.ping_wait)
		{
			g_net.ping_wait = true;
			g_net.ping_got = false;
			g_net.ping_ip = ip;
			*t0 = time_now_ns();
			rc = ping_send_l(ip);
			if (rc < 0)
				g_net.ping_wait = false;
		}
	}
	mutex_unlock(&g_net.lock);
	return (rc);
}

int	net_ping(uint32_t ip, uint64_t timeout_ns, uint64_t *rtt_ns)
{
	uint64_t	t0;
	int			rc;

	if (!g_net.ready)
		return (E_NODEV);
	t0 = 0;
	rc = ping_begin(ip, &t0);
	if (rc < 0)
		return (rc);
	rc = net_wait(ping_done, NULL, timeout_ns);
	mutex_lock(&g_net.lock);
	if (rc == 0 && rtt_ns)
		*rtt_ns = g_net.ping_t1 - t0;
	g_net.ping_wait = false;
	mutex_unlock(&g_net.lock);
	return (rc);
}
