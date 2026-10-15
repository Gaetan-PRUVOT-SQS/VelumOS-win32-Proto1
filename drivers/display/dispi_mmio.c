#include "display_int.h"

static uint16_t	mmio_read(void *ctx, uint16_t index)
{
	volatile uint16_t	*regs;

	regs = ctx;
	return (regs[index]);
}

static void	mmio_write(void *ctx, uint16_t index, uint16_t val)
{
	volatile uint16_t	*regs;

	regs = ctx;
	regs[index] = val;
}

void	dispi_mmio_bind(t_dispi *d, void *bar)
{
	d->ops.read = mmio_read;
	d->ops.write = mmio_write;
	d->ops.ctx = (uint8_t *)bar + DISPI_MMIO_REGS;
}
