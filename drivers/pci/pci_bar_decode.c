#include "pci_int.h"

static uint64_t	lowest_bit(uint64_t v)
{
	return (v & (~v + 1));
}

void	pcibar_decode(const uint32_t *v, const uint32_t *h, t_pcibar *b)
{
	b->slots = 1;
	if (v[0] & 1)
	{
		b->flags = PCI_BAR_IO;
		b->base = v[0] & ~3u;
		b->size = lowest_bit(v[1] & ~3u);
		return ;
	}
	b->base = v[0] & ~0xfu;
	b->size = lowest_bit(v[1] & ~0xfu);
	if (h)
	{
		b->base |= (uint64_t)h[0] << 32;
		b->size = lowest_bit(((uint64_t)h[1] << 32) | (v[1] & ~0xfu));
		b->flags |= PCI_BAR_MEM64;
		b->slots = 2;
	}
	if (v[0] & 8)
		b->flags |= PCI_BAR_PREFETCH;
}
