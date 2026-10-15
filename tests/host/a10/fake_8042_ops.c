#include "fake_8042.h"

void	f8042_write_cmd(uint8_t v)
{
	if (g_f8042.ncmds < F8_LOG)
		g_f8042.cmds[g_f8042.ncmds] = v;
	g_f8042.ncmds++;
	f8042_ctl_cmd(v);
}

void	f8042_write_data(uint8_t v)
{
	uint8_t	pending;

	pending = g_f8042.pending;
	g_f8042.pending = 0;
	if (pending == 0x60)
		g_f8042.cfg = v;
	else if (pending == 0xd4)
		f8042_mouse_in(v);
	else
		f8042_kbd_in(v);
}

uint32_t	f8042_pending(void)
{
	return (g_f8042.tail - g_f8042.head);
}

void	f8042_kbd_bytes(const uint8_t *b, uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		f8042_push(b[i], 0);
		i++;
	}
}

void	f8042_mouse_bytes(const uint8_t *b, uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		f8042_push(b[i], 1);
		i++;
	}
}
