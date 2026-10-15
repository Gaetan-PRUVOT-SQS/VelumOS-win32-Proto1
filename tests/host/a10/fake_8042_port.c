#include <string.h>
#include "fake_8042.h"

t_f8042	g_f8042;

void	f8042_reset(void)
{
	memset(&g_f8042, 0, sizeof(g_f8042));
	g_f8042.cfg = 0x47;
	g_f8042.kbd_present = 1;
	g_f8042.mouse_present = 1;
	g_f8042.dual = 1;
	g_f8042.wheel_ok = 1;
	g_f8042.selftest = 0x55;
	g_f8042.scanset = 2;
}

void	f8042_push(uint8_t b, uint8_t aux)
{
	uint32_t	i;

	if (g_f8042.tail - g_f8042.head >= F8_FIFO)
		return ;
	i = g_f8042.tail % F8_FIFO;
	g_f8042.out[i] = b;
	g_f8042.aux[i] = aux;
	g_f8042.err[i] = 0;
	g_f8042.tail++;
}

void	f8042_push_err(uint8_t b, uint8_t aux)
{
	f8042_push(b, aux);
	g_f8042.err[(g_f8042.tail - 1) % F8_FIFO] = 1;
}

uint8_t	f8042_status(void)
{
	uint8_t		st;
	uint32_t	i;

	if (g_f8042.float_bus)
		return (0xff);
	st = 0x04;
	if (g_f8042.stuck_ibf)
		st |= 0x02;
	if (g_f8042.head != g_f8042.tail)
	{
		i = g_f8042.head % F8_FIFO;
		st |= 0x01;
		if (g_f8042.aux[i])
			st |= 0x20;
		if (g_f8042.err[i])
			st |= 0x80;
	}
	return (st);
}

uint8_t	f8042_read(void)
{
	uint8_t	b;

	if (g_f8042.head == g_f8042.tail)
		return (0);
	b = g_f8042.out[g_f8042.head % F8_FIFO];
	g_f8042.head++;
	return (b);
}
