#include "net_wire.h"

uint32_t	net_sum(const uint8_t *p, size_t n, uint32_t acc)
{
	uint64_t	sum;
	size_t		i;

	sum = acc;
	i = 0;
	while (i + 1 < n)
	{
		sum += rd16(p + i);
		i += 2;
	}
	if (i < n)
		sum += (uint64_t)p[i] << 8;
	while (sum >> 32)
		sum = (sum & 0xffffffffu) + (sum >> 32);
	return ((uint32_t)sum);
}

uint16_t	net_fold(uint32_t acc)
{
	while (acc >> 16)
		acc = (acc & 0xffffu) + (acc >> 16);
	return ((uint16_t)~acc);
}

uint16_t	net_cksum(const uint8_t *p, size_t n)
{
	return (net_fold(net_sum(p, n, 0)));
}
