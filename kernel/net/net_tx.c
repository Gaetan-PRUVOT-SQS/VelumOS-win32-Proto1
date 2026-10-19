#include "velum/err.h"
#include "velum/libk.h"
#include "net_int.h"

int	net_tx_l(const uint8_t mac[6], uint32_t dst, uint8_t proto,
		const uint8_t *l4, size_t l4len)
{
	t_ipinfo	h;
	size_t		n;

	if (!g_net.nic || l4len > NET_FRAME_MAX - ETH_HLEN - IP_HLEN)
		return (E_INVAL);
	h.src = g_net.ip;
	h.dst = dst;
	h.proto = proto;
	h.ttl = IP_TTL;
	h.id = g_net.ip_id++;
	h.len = l4len;
	n = eth_build(g_net.tx, mac, g_net.mac, ETH_P_IP);
	n += ip_build(g_net.tx + n, &h);
	memcpy(g_net.tx + n, l4, l4len);
	n += l4len;
	if (n < NET_FRAME_MIN)
	{
		memset(g_net.tx + n, 0, NET_FRAME_MIN - n);
		n = NET_FRAME_MIN;
	}
	return (g_net.nic->send(g_net.nic_ctx, g_net.tx, n));
}

int	net_route_l(uint32_t dst, uint8_t mac[6])
{
	uint32_t	hop;

	if (dst == NET_IP_BROADCAST)
	{
		memset(mac, 0xff, 6);
		return (0);
	}
	if (g_net.ip == 0)
		return (E_NODEV);
	hop = dst;
	if ((dst & g_net.mask) != (g_net.ip & g_net.mask))
		hop = g_net.gw;
	if (hop == 0)
		return (E_NOENT);
	return (arp_resolve_l(hop, mac));
}

int	net_udp_send_l(uint16_t sport, uint32_t dst, uint16_t dport,
		const uint8_t *data, size_t len)
{
	uint8_t		mac[6];
	t_ipinfo	pseudo;
	t_udpinfo	u;
	int			rc;

	if (len > NET_UDP_MAX)
		return (E_INVAL);
	rc = net_route_l(dst, mac);
	if (rc < 0)
		return (rc);
	pseudo.src = g_net.ip;
	pseudo.dst = dst;
	u.sport = sport;
	u.dport = dport;
	u.data = data;
	u.len = len;
	return (net_tx_l(mac, dst, IP_PROTO_UDP, g_net.l4,
			udp_build(g_net.l4, &pseudo, &u)));
}
