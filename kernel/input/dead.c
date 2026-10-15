#include "kbd.h"

static const t_compose	g_compose[] = {
{0x5e, 'a', 0xe2}, {0x5e, 'e', 0xea}, {0x5e, 'i', 0xee},
{0x5e, 'o', 0xf4}, {0x5e, 'u', 0xfb}, {0x5e, 'A', 0xc2},
{0x5e, 'E', 0xca}, {0x5e, 'I', 0xce}, {0x5e, 'O', 0xd4},
{0x5e, 'U', 0xdb},
{0xa8, 'a', 0xe4}, {0xa8, 'e', 0xeb}, {0xa8, 'i', 0xef},
{0xa8, 'o', 0xf6}, {0xa8, 'u', 0xfc}, {0xa8, 'y', 0xff},
{0xa8, 'A', 0xc4}, {0xa8, 'E', 0xcb}, {0xa8, 'I', 0xcf},
{0xa8, 'O', 0xd6}, {0xa8, 'U', 0xdc}, {0xa8, 'Y', 0x178},
{0x60, 'a', 0xe0}, {0x60, 'e', 0xe8}, {0x60, 'i', 0xec},
{0x60, 'o', 0xf2}, {0x60, 'u', 0xf9}, {0x60, 'A', 0xc0},
{0x60, 'E', 0xc8}, {0x60, 'I', 0xcc}, {0x60, 'O', 0xd2},
{0x60, 'U', 0xd9},
{0x7e, 'a', 0xe3}, {0x7e, 'n', 0xf1}, {0x7e, 'o', 0xf5},
{0x7e, 'A', 0xc3}, {0x7e, 'N', 0xd1}, {0x7e, 'O', 0xd5},
};

uint32_t	dead_compose(uint32_t accent, uint32_t base)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_compose) / sizeof(g_compose[0]))
	{
		if (g_compose[i].accent == accent && g_compose[i].base == base)
			return (g_compose[i].result);
		i++;
	}
	return (0);
}

static int	dead_press(t_kbd *k, uint32_t accent, uint32_t *out)
{
	if (k->dead == 0)
	{
		k->dead = accent;
		return (0);
	}
	out[0] = k->dead;
	if (k->dead == accent)
		k->dead = 0;
	else
		k->dead = accent;
	return (1);
}

int	dead_apply(t_kbd *k, uint32_t ch, uint32_t *out)
{
	uint32_t	accent;
	uint32_t	comp;

	if (ch & K_DEAD)
		return (dead_press(k, ch & ~K_DEAD, out));
	if (k->dead == 0)
	{
		out[0] = ch;
		return (1);
	}
	accent = k->dead;
	k->dead = 0;
	comp = dead_compose(accent, ch);
	if (comp)
		out[0] = comp;
	else
		out[0] = accent;
	if (comp || ch == ' ')
		return (1);
	out[1] = ch;
	return (2);
}
