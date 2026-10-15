#include "ps2.h"
#include "velum/io.h"

static uint8_t	hw_status(void)
{
	return (inb(PS2_PORT_CMD));
}

static uint8_t	hw_read(void)
{
	return (inb(PS2_PORT_DATA));
}

static void	hw_write_cmd(uint8_t v)
{
	outb(PS2_PORT_CMD, v);
}

static void	hw_write_data(uint8_t v)
{
	outb(PS2_PORT_DATA, v);
}

static const t_ps2ops	g_hw_ops = {hw_status, hw_read, hw_write_cmd,
	hw_write_data};

const t_ps2ops	*ps2_hw_ops(void)
{
	return (&g_hw_ops);
}
