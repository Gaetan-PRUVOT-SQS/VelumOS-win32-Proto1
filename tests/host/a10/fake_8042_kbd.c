#include "fake_8042.h"

static void	kbd_log(uint8_t v)
{
	if (g_f8042.nkbd < F8_LOG)
		g_f8042.kbd_log[g_f8042.nkbd] = v;
	g_f8042.nkbd++;
}

static void	kbd_take_arg(uint8_t v)
{
	if (g_f8042.kbd_arg == 0xed)
		g_f8042.leds = v & 7;
	else if (g_f8042.kbd_arg == 0xf0 && v != 0)
		g_f8042.scanset = v;
	f8042_push(0xfa, 0);
	if (g_f8042.kbd_arg == 0xf0 && v == 0)
		f8042_push(g_f8042.scanset, 0);
	g_f8042.kbd_arg = 0;
}

static void	kbd_command(uint8_t v)
{
	if (v == 0xed || v == 0xf0 || v == 0xf3)
		g_f8042.kbd_arg = v;
	if (v == 0xff)
	{
		f8042_push(0xfa, 0);
		f8042_push(0xaa, 0);
		g_f8042.scanset = 2;
	}
	else if (v == 0xee)
		f8042_push(0xee, 0);
	else if (v == 0xed || v == 0xf0 || v == 0xf3 || v == 0xf4 || v == 0xf5
		|| v == 0xf6)
		f8042_push(0xfa, 0);
	else
		f8042_push(0xfe, 0);
	if (v == 0xf4)
		g_f8042.kbd_scanning = 1;
	if (v == 0xf5)
		g_f8042.kbd_scanning = 0;
}

void	f8042_kbd_in(uint8_t v)
{
	kbd_log(v);
	if (!g_f8042.kbd_present || g_f8042.kbd_silent || (g_f8042.cfg & 0x10))
		return ;
	if (g_f8042.nak_kbd)
	{
		g_f8042.nak_kbd--;
		f8042_push(0xfe, 0);
		return ;
	}
	if (g_f8042.kbd_arg)
		kbd_take_arg(v);
	else
		kbd_command(v);
}
