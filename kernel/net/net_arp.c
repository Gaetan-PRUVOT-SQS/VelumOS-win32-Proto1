#include "velum/err.h"
#include "velum/libk.h"
#include "velum/timer.h"
#include "net_int.h"

static bool	valid_peer(uint32_t ip, const uint8_t mac[6])
{
	return (ip != 0 && ip != NET_IP_BROADCAST && !(mac[0] & 1));
}

static t_arpent	*arp_slot(uint32_t ip, uint64_t now)
{
	t_arpent	*oldest;
	uint32_t	i;

	oldest = &g_net.arp[0];
	i = 0;
	while (i < NET_ARP_SLOTS)
	{
		if (g_net.arp[i].used && g_net.arp[i].ip == ip)
			return (&g_net.arp[i]);
		if (!g_net.arp[i].used || g_net.arp[i].expire_ns <= now)
			return (&g_net.arp[i]);
		if (g_net.arp[i].expire_ns < oldest->expire_ns)
			oldest = &g_net.arp[i];
		i++;
	}
	return (oldest);
}

static void	arp_learn_l(uint32_t ip, const uint8_t mac[6])
{
	uint64_t	now;
	t_arpent	*s;

	now = time_now_ns();
	s = arp_slot(ip, now);
	s->used = true;
	s->ip = ip;
	memcpy(s->mac, mac, 6);
	s->expire_ns = now + NET_ARP_TTL_NS;
}

bool	arp_lookup_l(uint32_t ip, uint8_t mac[6])
{
	uint64_t	now;
	uint32_t	i;

	now = time_now_ns();
	i = 0;
	while (i < NET_ARP_SLOTS)
	{
		if (g_net.arp[i].used && g_net.arp[i].ip == ip
			&& g_net.arp[i].expire_ns > now)
		{
			memcpy(mac, g_net.arp[i].mac, 6);
			return (true);
		}
		i++;
	}
	return (false);
}

static void	arp_send_l(uint16_t op, const uint8_t dst_mac[6], uint32_t tpa)
{
	static const uint8_t	zero[6] = {0, 0, 0, 0, 0, 0};
	t_arpinfo				a;
	size_t					n;

	a.op = op;
	memcpy(a.sha, g_net.mac, 6);
	a.spa = g_net.ip;
	memcpy(a.tha, op == ARP_REQUEST ? zero : dst_mac, 6);
	a.tpa = tpa;
	n = eth_build(g_net.tx, dst_mac, g_net.mac, ETH_P_ARP);
	n += arp_build(g_net.tx + n, &a);
	memset(g_net.tx + n, 0, NET_FRAME_MIN - n);
	g_net.nic->send(g_net.nic_ctx, g_net.tx, NET_FRAME_MIN);
}

void	arp_input_l(const t_ethinfo *e)
{
	t_arpinfo	a;

	if (arp_parse(e->data, e->len, &a) < 0 || !valid_peer(a.spa, a.sha))
		return ;
	if (a.op == ARP_REPLY && a.spa == g_net.arp_wait_ip)
		arp_learn_l(a.spa, a.sha);
	if (a.op != ARP_REQUEST || g_net.ip == 0 || a.tpa != g_net.ip)
		return ;
	arp_learn_l(a.spa, a.sha);
	arp_send_l(ARP_REPLY, a.sha, a.spa);
}

static bool	arp_known(void *ctx)
{
	uint8_t	mac[6];

	(void)ctx;
	net_poll_l();
	return (arp_lookup_l(g_net.arp_wait_ip, mac));
}

int	arp_resolve_l(uint32_t ip, uint8_t mac[6])
{
	static const uint8_t	bcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	int						tries;

	if (arp_lookup_l(ip, mac))
		return (0);
	g_net.arp_wait_ip = ip;
	tries = 0;
	while (tries < NET_ARP_TRIES)
	{
		arp_send_l(ARP_REQUEST, bcast, ip);
		if (wait_until(arp_known, NULL, NET_ARP_TRY_NS) == 0)
			break ;
		tries++;
	}
	g_net.arp_wait_ip = 0;
	if (arp_lookup_l(ip, mac))
		return (0);
	return (E_TIMEOUT);
}
