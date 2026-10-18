#include "velum/err.h"
#include "velum/libk.h"
#include "net_wire.h"

int	eth_parse(const uint8_t *f, size_t n, t_ethinfo *out)
{
	if (!f || !out || n < ETH_HLEN)
		return (E_INVAL);
	memcpy(out->dst, f, 6);
	memcpy(out->src, f + 6, 6);
	out->type = rd16(f + 12);
	out->data = f + ETH_HLEN;
	out->len = n - ETH_HLEN;
	return (0);
}

size_t	eth_build(uint8_t *buf, const uint8_t dst[6], const uint8_t src[6],
		uint16_t type)
{
	memcpy(buf, dst, 6);
	memcpy(buf + 6, src, 6);
	wr16(buf + 12, type);
	return (ETH_HLEN);
}

int	arp_parse(const uint8_t *p, size_t n, t_arpinfo *out)
{
	if (!p || !out || n < ARP_LEN)
		return (E_INVAL);
	if (rd16(p) != 1 || rd16(p + 2) != ETH_P_IP || p[4] != 6 || p[5] != 4)
		return (E_INVAL);
	out->op = rd16(p + 6);
	if (out->op != ARP_REQUEST && out->op != ARP_REPLY)
		return (E_INVAL);
	memcpy(out->sha, p + 8, 6);
	out->spa = rd32(p + 14);
	memcpy(out->tha, p + 18, 6);
	out->tpa = rd32(p + 24);
	return (0);
}

size_t	arp_build(uint8_t *buf, const t_arpinfo *a)
{
	wr16(buf, 1);
	wr16(buf + 2, ETH_P_IP);
	buf[4] = 6;
	buf[5] = 4;
	wr16(buf + 6, a->op);
	memcpy(buf + 8, a->sha, 6);
	wr32(buf + 14, a->spa);
	memcpy(buf + 18, a->tha, 6);
	wr32(buf + 24, a->tpa);
	return (ARP_LEN);
}
