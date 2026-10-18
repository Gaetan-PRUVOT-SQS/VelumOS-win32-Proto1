#include <string.h>
#include "velum/err.h"
#include "fake.h"
#include "net_int.h"

const uint8_t	g_mac_local[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
const uint8_t	g_mac_peer[6] = {0x52, 0x55, 0x0a, 0x00, 0x02, 0x02};

static uint8_t	g_rx[FAKE_FRAMES][NET_FRAME_MAX];
static size_t	g_rx_len[FAKE_FRAMES];
static int		g_rx_head;
static int		g_rx_count;
static uint8_t	g_tx[FAKE_FRAMES][NET_FRAME_MAX];
static size_t	g_tx_len[FAKE_FRAMES];
static int		g_tx_count;
static t_peerfn	g_peer;

void	fake_nic_inject(const uint8_t *frame, size_t len)
{
	int	slot;

	if (g_rx_count == FAKE_FRAMES)
		return ;
	slot = (g_rx_head + g_rx_count) % FAKE_FRAMES;
	memcpy(g_rx[slot], frame, len);
	g_rx_len[slot] = len;
	g_rx_count++;
}

static int	nic_send(void *ctx, const uint8_t *frame, size_t len)
{
	(void)ctx;
	if (g_tx_count < FAKE_FRAMES)
	{
		memcpy(g_tx[g_tx_count], frame, len);
		g_tx_len[g_tx_count] = len;
		g_tx_count++;
	}
	if (g_peer)
		g_peer(frame, len);
	return (0);
}

static int	nic_recv(void *ctx, uint8_t *frame, size_t cap)
{
	size_t	n;

	(void)ctx;
	if (g_rx_count == 0)
		return (0);
	n = g_rx_len[g_rx_head];
	if (n > cap)
		n = cap;
	memcpy(frame, g_rx[g_rx_head], n);
	g_rx_head = (g_rx_head + 1) % FAKE_FRAMES;
	g_rx_count--;
	return ((int)n);
}

static const t_nicops	g_ops = {nic_send, nic_recv};

void	fake_nic_set_peer(t_peerfn fn)
{
	g_peer = fn;
}

int	fake_nic_tx_count(void)
{
	return (g_tx_count);
}

const uint8_t	*fake_nic_tx(int i, size_t *len)
{
	*len = g_tx_len[i];
	return (g_tx[i]);
}

void	fake_reset(void)
{
	net_detach();
	g_rx_head = 0;
	g_rx_count = 0;
	g_tx_count = 0;
	g_peer = NULL;
	fake_fail_alloc(-1);
}

void	fake_nic_attach(void)
{
	fake_reset();
	net_attach(&g_ops, NULL, g_mac_local);
}

size_t	fake_udp_frame(uint8_t *buf, uint32_t src, uint32_t dst,
		uint16_t sport, uint16_t dport, const uint8_t *data, size_t n)
{
	t_ipinfo	ip;
	t_udpinfo	u;
	size_t		len;

	ip.src = src;
	ip.dst = dst;
	ip.proto = IP_PROTO_UDP;
	ip.ttl = 64;
	ip.id = 1;
	u.sport = sport;
	u.dport = dport;
	u.data = data;
	u.len = n;
	len = udp_build(buf + ETH_HLEN + IP_HLEN, &ip, &u);
	ip.len = len;
	eth_build(buf, g_mac_local, g_mac_peer, ETH_P_IP);
	ip_build(buf + ETH_HLEN, &ip);
	return (ETH_HLEN + IP_HLEN + len);
}

size_t	fake_icmp_frame(uint8_t *buf, uint32_t src, uint32_t dst,
		uint8_t type, uint16_t seq, const uint8_t *data, size_t n)
{
	t_ipinfo	ip;
	t_icmpinfo	m;
	size_t		len;

	m.type = type;
	m.id = 0x5656;
	m.seq = seq;
	m.data = data;
	m.len = n;
	len = icmp_build(buf + ETH_HLEN + IP_HLEN, &m);
	ip.src = src;
	ip.dst = dst;
	ip.proto = IP_PROTO_ICMP;
	ip.ttl = 64;
	ip.id = 2;
	ip.len = len;
	eth_build(buf, g_mac_local, g_mac_peer, ETH_P_IP);
	ip_build(buf + ETH_HLEN, &ip);
	return (ETH_HLEN + IP_HLEN + len);
}
