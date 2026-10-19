#include "velum/err.h"
#include "velum/libk.h"
#include "net_wire.h"

#define IP_FLAG_MF 0x2000
#define IP_FLAG_DF 0x4000
#define IP_OFFSET_MASK 0x1fff

int	ip_parse(const uint8_t *p, size_t n, t_ipinfo *out)
{
	size_t	ihl;
	size_t	total;

	if (!p || !out || n < IP_HLEN || (p[0] >> 4) != 4)
		return (E_INVAL);
	ihl = (size_t)(p[0] & 0x0f) * 4;
	total = rd16(p + 2);
	if (ihl < IP_HLEN || ihl > n || total < ihl || total > n)
		return (E_INVAL);
	if (net_cksum(p, ihl) != 0)
		return (E_INVAL);
	if (rd16(p + 6) & (IP_FLAG_MF | IP_OFFSET_MASK))
		return (E_NOTSUP);
	out->id = rd16(p + 4);
	out->ttl = p[8];
	out->proto = p[9];
	out->src = rd32(p + 12);
	out->dst = rd32(p + 16);
	out->data = p + ihl;
	out->len = total - ihl;
	return (0);
}

size_t	ip_build(uint8_t *buf, const t_ipinfo *h)
{
	buf[0] = 0x45;
	buf[1] = 0;
	wr16(buf + 2, (uint16_t)(IP_HLEN + h->len));
	wr16(buf + 4, h->id);
	wr16(buf + 6, IP_FLAG_DF);
	buf[8] = h->ttl;
	buf[9] = h->proto;
	wr16(buf + 10, 0);
	wr32(buf + 12, h->src);
	wr32(buf + 16, h->dst);
	wr16(buf + 10, net_cksum(buf, IP_HLEN));
	return (IP_HLEN);
}

int	icmp_parse(const uint8_t *p, size_t n, t_icmpinfo *out)
{
	if (!p || !out || n < ICMP_HLEN)
		return (E_INVAL);
	if (p[1] != 0 || (p[0] != ICMP_ECHO_REQUEST && p[0] != ICMP_ECHO_REPLY))
		return (E_INVAL);
	if (net_cksum(p, n) != 0)
		return (E_INVAL);
	out->type = p[0];
	out->id = rd16(p + 4);
	out->seq = rd16(p + 6);
	out->data = p + ICMP_HLEN;
	out->len = n - ICMP_HLEN;
	return (0);
}

size_t	icmp_build(uint8_t *buf, const t_icmpinfo *e)
{
	buf[0] = e->type;
	buf[1] = 0;
	wr16(buf + 2, 0);
	wr16(buf + 4, e->id);
	wr16(buf + 6, e->seq);
	memcpy(buf + ICMP_HLEN, e->data, e->len);
	wr16(buf + 2, net_cksum(buf, ICMP_HLEN + e->len));
	return (ICMP_HLEN + e->len);
}

static uint32_t	udp_pseudo(const t_ipinfo *ip, size_t udp_len)
{
	uint8_t	ph[12];

	wr32(ph, ip->src);
	wr32(ph + 4, ip->dst);
	ph[8] = 0;
	ph[9] = IP_PROTO_UDP;
	wr16(ph + 10, (uint16_t)udp_len);
	return (net_sum(ph, sizeof(ph), 0));
}

int	udp_parse(const t_ipinfo *ip, t_udpinfo *out)
{
	const uint8_t	*p;
	size_t			len;

	if (!ip || !out || ip->proto != IP_PROTO_UDP || ip->len < UDP_HLEN)
		return (E_INVAL);
	p = ip->data;
	len = rd16(p + 4);
	if (len < UDP_HLEN || len > ip->len)
		return (E_INVAL);
	if (rd16(p + 6) != 0
		&& net_fold(net_sum(p, len, udp_pseudo(ip, len))) != 0)
		return (E_INVAL);
	out->sport = rd16(p);
	out->dport = rd16(p + 2);
	out->data = p + UDP_HLEN;
	out->len = len - UDP_HLEN;
	return (0);
}

size_t	udp_build(uint8_t *buf, const t_ipinfo *ip, const t_udpinfo *u)
{
	size_t		len;
	uint16_t	sum;

	len = UDP_HLEN + u->len;
	wr16(buf, u->sport);
	wr16(buf + 2, u->dport);
	wr16(buf + 4, (uint16_t)len);
	wr16(buf + 6, 0);
	memcpy(buf + UDP_HLEN, u->data, u->len);
	sum = net_fold(net_sum(buf, len, udp_pseudo(ip, len)));
	if (sum == 0)
		sum = 0xffff;
	wr16(buf + 6, sum);
	return (len);
}
