#include "display_int.h"
#include "velum/io.h"

static uint16_t	port_read(void *ctx, uint16_t index)
{
	(void)ctx;
	outw(DISPI_PORT_INDEX, index);
	return (inw(DISPI_PORT_DATA));
}

static void	port_write(void *ctx, uint16_t index, uint16_t val)
{
	(void)ctx;
	outw(DISPI_PORT_INDEX, index);
	outw(DISPI_PORT_DATA, val);
}

void	dispi_io_bind(t_dispi *d)
{
	d->ops.read = port_read;
	d->ops.write = port_write;
	d->ops.ctx = NULL;
}
