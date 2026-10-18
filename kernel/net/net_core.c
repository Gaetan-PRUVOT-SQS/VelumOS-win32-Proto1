#include "velum/err.h"
#include "velum/libk.h"
#include "velum/timer.h"
#include "net_int.h"

t_net	g_net;

int	net_attach(const t_nicops *ops, void *ctx, const uint8_t mac[6])
{
	if (!ops || !ops->send || !ops->recv || !mac)
		return (E_INVAL);
	if (!g_net.ready)
	{
		memset(&g_net, 0, sizeof(g_net));
		mutex_init(&g_net.lock, "net");
		g_net.ready = true;
	}
	mutex_lock(&g_net.lock);
	if (g_net.nic)
	{
		mutex_unlock(&g_net.lock);
		return (E_BUSY);
	}
	g_net.nic = ops;
	g_net.nic_ctx = ctx;
	memcpy(g_net.mac, mac, 6);
	g_net.etat = NET_ETAT_SANS_ADRESSE;
	mutex_unlock(&g_net.lock);
	return (0);
}

void	net_detach(void)
{
	uint32_t	i;

	if (!g_net.ready)
		return ;
	mutex_lock(&g_net.lock);
	i = 0;
	while (i < NET_SOCKS)
	{
		if (g_net.socks[i].used)
			sock_free_l(&g_net.socks[i]);
		i++;
	}
	g_net.nic = NULL;
	g_net.nic_ctx = NULL;
	g_net.etat = NET_ETAT_ABSENT;
	g_net.ip = 0;
	g_net.mask = 0;
	g_net.gw = 0;
	g_net.dns = 0;
	memset(g_net.arp, 0, sizeof(g_net.arp));
	mutex_unlock(&g_net.lock);
}

void	net_info(t_netinfo *out)
{
	memset(out, 0, sizeof(*out));
	if (!g_net.ready)
		return ;
	mutex_lock(&g_net.lock);
	out->etat = g_net.etat;
	out->ip = g_net.ip;
	out->masque = g_net.mask;
	out->passerelle = g_net.gw;
	out->dns = g_net.dns;
	out->bail_s = g_net.lease_s;
	memcpy(out->mac, g_net.mac, 6);
	mutex_unlock(&g_net.lock);
}

static bool	for_us(const t_ethinfo *e)
{
	static const uint8_t	bcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	return (memcmp(e->dst, g_net.mac, 6) == 0 || memcmp(e->dst, bcast, 6) == 0);
}

static void	udp_input_l(const t_ipinfo *ip)
{
	t_udpinfo	u;

	if (udp_parse(ip, &u) < 0)
	{
		g_net.drops++;
		return ;
	}
	if (!sock_deliver_l(u.dport, ip->src, u.sport, u.data, u.len))
		g_net.drops++;
}

static void	ip_input_l(const t_ethinfo *e)
{
	t_ipinfo	ip;

	if (ip_parse(e->data, e->len, &ip) < 0 || ip.ttl == 0)
	{
		g_net.drops++;
		return ;
	}
	if (ip.dst != g_net.ip && ip.dst != NET_IP_BROADCAST)
		return ;
	if (g_net.ip == 0 && ip.proto != IP_PROTO_UDP)
		return ;
	if (ip.proto == IP_PROTO_ICMP)
		icmp_input_l(e, &ip);
	else if (ip.proto == IP_PROTO_UDP)
		udp_input_l(&ip);
}

void	net_rx_l(const uint8_t *frame, size_t len)
{
	t_ethinfo	e;

	if (eth_parse(frame, len, &e) < 0 || !for_us(&e))
		return ;
	if (e.type == ETH_P_ARP)
		arp_input_l(&e);
	else if (e.type == ETH_P_IP)
		ip_input_l(&e);
}

void	net_poll_l(void)
{
	int	i;
	int	n;

	if (!g_net.nic)
		return ;
	i = 0;
	while (i < NET_RX_BURST)
	{
		n = g_net.nic->recv(g_net.nic_ctx, g_net.rx, sizeof(g_net.rx));
		if (n <= 0)
			break ;
		net_rx_l(g_net.rx, (size_t)n);
		i++;
	}
}

void	net_poll(void)
{
	if (!g_net.ready)
		return ;
	mutex_lock(&g_net.lock);
	net_poll_l();
	mutex_unlock(&g_net.lock);
}

typedef struct s_netwait
{
	t_nettest	test;
	void		*arg;
}	t_netwait;

static bool	netwait_cond(void *ctx)
{
	t_netwait	*w;
	bool		ok;

	w = ctx;
	if (!mutex_trylock(&g_net.lock))
		return (false);
	net_poll_l();
	ok = w->test(w->arg);
	mutex_unlock(&g_net.lock);
	return (ok);
}

int	net_wait(t_nettest test, void *arg, uint64_t timeout_ns)
{
	t_netwait	w;

	w.test = test;
	w.arg = arg;
	return (wait_until(netwait_cond, &w, timeout_ns));
}
