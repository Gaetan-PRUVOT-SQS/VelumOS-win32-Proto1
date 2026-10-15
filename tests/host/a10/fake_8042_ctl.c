#include "fake_8042.h"

static void	ctl_port_toggle(uint8_t c)
{
	if (c == 0xa7)
		g_f8042.cfg |= 0x20;
	else if (c == 0xa8 && g_f8042.dual)
		g_f8042.cfg &= 0xdf;
	else if (c == 0xad)
		g_f8042.cfg |= 0x10;
	else if (c == 0xae)
		g_f8042.cfg &= 0xef;
}

static void	ctl_query(uint8_t c)
{
	if (c == 0x20)
		f8042_push(g_f8042.cfg, 0);
	else if (c == 0xaa)
	{
		f8042_push(g_f8042.selftest, 0);
		if (g_f8042.selftest_resets)
			g_f8042.cfg = 0x45;
	}
	else if (c == 0xab)
		f8042_push(g_f8042.porttest[0], 0);
	else if (c == 0xa9 && g_f8042.dual)
		f8042_push(g_f8042.porttest[1], 0);
}

void	f8042_ctl_cmd(uint8_t c)
{
	if (c == 0x60 || c == 0xd4)
		g_f8042.pending = c;
	ctl_port_toggle(c);
	ctl_query(c);
}
