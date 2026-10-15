#include "kbd.h"

static const t_modmap	g_modmap[] = {
{0x12, KM_LSHIFT, 0}, {0x59, KM_RSHIFT, 0},
{0x14, KM_LCTRL, 0}, {0xe014, KM_RCTRL, 0},
{0x11, KM_LALT, 0}, {0xe011, KM_RALT, 0},
{0xe01f, KM_LWIN, 0}, {0xe027, KM_RWIN, 0},
{0x58, 0, INPUT_LED_CAPS}, {0x77, 0, INPUT_LED_NUM},
{0x7e, 0, INPUT_LED_SCROLL},
};

static const t_modmap	*modmap_find(uint16_t code)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_modmap) / sizeof(g_modmap[0]))
	{
		if (g_modmap[i].code == code)
			return (&g_modmap[i]);
		i++;
	}
	return (0);
}

int	kbd_mod_apply(t_kbd *k, const t_keyraw *raw)
{
	const t_modmap	*m;

	m = modmap_find(raw->code);
	if (!m)
		return (0);
	if (m->held && raw->release)
		k->held ^= (uint8_t)(k->held & m->held);
	else if (m->held)
		k->held |= m->held;
	if (m->lock && !raw->release && !raw->repeat)
	{
		k->locks ^= m->lock;
		return (1);
	}
	return (0);
}

uint32_t	kbd_mods(const t_kbd *k)
{
	uint32_t	mods;

	mods = 0;
	if (k->held & KM_SHIFT)
		mods |= INPM_SHIFT;
	if (k->held & KM_CTRL)
		mods |= INPM_CTRL;
	if (k->held & KM_ALT)
		mods |= INPM_ALT;
	if (k->held & KM_WIN)
		mods |= INPM_WIN;
	if (k->locks & INPUT_LED_CAPS)
		mods |= INPM_CAPS;
	if (k->locks & INPUT_LED_NUM)
		mods |= INPM_NUM;
	return (mods);
}

int	kbd_altgr(const t_kbd *k)
{
	if (!k->layout->altgr)
		return (0);
	if (k->held & KM_RALT)
		return (1);
	return ((k->held & KM_CTRL) && (k->held & KM_ALT));
}

int	kbd_numpad(const t_kbd *k)
{
	int	num;
	int	shift;

	num = (k->locks & INPUT_LED_NUM) != 0;
	shift = (k->held & KM_SHIFT) != 0;
	return (num != shift);
}
