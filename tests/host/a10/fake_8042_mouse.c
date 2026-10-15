#include "fake_8042.h"

static void	mouse_log(uint8_t v)
{
	if (g_f8042.nmouse < F8_LOG)
		g_f8042.mouse_log[g_f8042.nmouse] = v;
	g_f8042.nmouse++;
}

static void	mouse_take_arg(uint8_t v)
{
	if (g_f8042.mouse_arg == 0xf3)
	{
		g_f8042.rates[0] = g_f8042.rates[1];
		g_f8042.rates[1] = g_f8042.rates[2];
		g_f8042.rates[2] = v;
		if (g_f8042.wheel_ok && g_f8042.rates[0] == 200
			&& g_f8042.rates[1] == 100 && g_f8042.rates[2] == 80)
			g_f8042.mouse_id = 3;
	}
	g_f8042.mouse_arg = 0;
	f8042_push(0xfa, 1);
}

static void	mouse_command(uint8_t v)
{
	if (v == 0xff)
	{
		f8042_push(0xfa, 1);
		f8042_push(0xaa, 1);
		f8042_push(0x00, 1);
		g_f8042.mouse_id = 0;
		g_f8042.mouse_stream = 0;
	}
	else if (v == 0xf2)
	{
		f8042_push(0xfa, 1);
		f8042_push(g_f8042.mouse_id, 1);
	}
	else if (v == 0xf3 || v == 0xe8)
		g_f8042.mouse_arg = v;
	if (v == 0xf3 || v == 0xe8 || v == 0xf4 || v == 0xf5 || v == 0xf6)
		f8042_push(0xfa, 1);
	if (v == 0xf4)
		g_f8042.mouse_stream = 1;
	if (v == 0xf5 || v == 0xf6)
		g_f8042.mouse_stream = 0;
}

void	f8042_mouse_in(uint8_t v)
{
	mouse_log(v);
	if (!g_f8042.mouse_present || g_f8042.mouse_silent || (g_f8042.cfg & 0x20))
		return ;
	if (g_f8042.mouse_arg)
		mouse_take_arg(v);
	else
		mouse_command(v);
}
