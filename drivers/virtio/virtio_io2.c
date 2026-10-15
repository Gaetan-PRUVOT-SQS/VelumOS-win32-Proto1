#include "virtio.h"

void	vio_wr32(volatile uint8_t *base, uint32_t off, uint32_t v)
{
	*(volatile uint32_t *)(base + off) = v;
}

void	vio_wr64(volatile uint8_t *base, uint32_t off, uint64_t v)
{
	vio_wr32(base, off, (uint32_t)v);
	vio_wr32(base, off + 4, (uint32_t)(v >> 32));
}
