#include "kbd.h"

static const uint16_t	g_held_code[KBD_HELD_MAX] = {
	0x12, 0x59, 0x14, 0xe014, 0x11, 0xe011, 0xe01f, 0xe027,
};

int	kbd_release_held(t_kbd *k, t_inpevent *out)
{
	t_keyraw	raw;
	uint32_t	i;
	int			n;

	n = 0;
	i = 0;
	raw.release = 1;
	raw.repeat = 0;
	while (i < KBD_HELD_MAX)
	{
		raw.code = g_held_code[i];
		if (k->held & (1u << i))
			n += kbd_translate(k, &raw, &out[n]);
		i++;
	}
	k->held = 0;
	k->dead = 0;
	return (n);
}

int	kbd_ghost_release(const t_kbd *k, const t_keyraw *raw)
{
	uint32_t	i;

	if (!raw->release)
		return (0);
	i = 0;
	while (i < KBD_HELD_MAX)
	{
		if (g_held_code[i] == raw->code)
			return (!(k->held & (1u << i)));
		i++;
	}
	return (0);
}
