#include "virtio.h"

uint8_t	vio_rd8(volatile uint8_t *base, uint32_t off)
{
	return (*(volatile uint8_t *)(base + off));
}

uint16_t	vio_rd16(volatile uint8_t *base, uint32_t off)
{
	return (*(volatile uint16_t *)(base + off));
}

uint32_t	vio_rd32(volatile uint8_t *base, uint32_t off)
{
	return (*(volatile uint32_t *)(base + off));
}

void	vio_wr8(volatile uint8_t *base, uint32_t off, uint8_t v)
{
	*(volatile uint8_t *)(base + off) = v;
}

void	vio_wr16(volatile uint8_t *base, uint32_t off, uint16_t v)
{
	*(volatile uint16_t *)(base + off) = v;
}
