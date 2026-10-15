#include "kbd.h"
#include "velum/libk.h"

void	kbd_init(t_kbd *k)
{
	memset(k, 0, sizeof(*k));
	k->layout = layout_default();
	k->locks = INPUT_LED_NUM;
}

static void	key_event(const t_kbd *k, const t_key *key, const t_keyraw *raw,
		t_inpevent *ev)
{
	memset(ev, 0, sizeof(*ev));
	ev->type = INP_KEY_DOWN;
	if (raw->release)
		ev->type = INP_KEY_UP;
	ev->code = key->vk;
	if ((key->flags & KF_PAD) && kbd_numpad(k))
		ev->code = key->vk_num;
	ev->mods = kbd_mods(k);
	if (raw->repeat)
		ev->mods |= INPM_REPEAT;
	if ((raw->code >> 8) == 0xe0)
		ev->mods |= INPM_EXTENDED;
	ev->scancode = raw->code;
}

static void	char_event(const t_kbd *k, const t_keyraw *raw, uint32_t ch,
		t_inpevent *ev)
{
	memset(ev, 0, sizeof(*ev));
	ev->type = INP_CHAR;
	ev->code = ch;
	ev->mods = kbd_mods(k);
	if (kbd_altgr(k))
		ev->mods ^= ev->mods & (INPM_CTRL | INPM_ALT);
	if (raw->repeat)
		ev->mods |= INPM_REPEAT;
	ev->scancode = raw->code;
}

static int	char_events(t_kbd *k, const t_key *key, const t_keyraw *raw,
		t_inpevent *out)
{
	uint32_t	chars[2];
	uint32_t	ch;
	int			cnt;
	int			i;

	ch = kbd_resolve(k, key);
	if (ch == 0)
		return (0);
	cnt = dead_apply(k, ch, chars);
	i = 0;
	while (i < cnt)
	{
		char_event(k, raw, chars[i], &out[i]);
		i++;
	}
	return (cnt);
}

int	kbd_translate(t_kbd *k, const t_keyraw *raw, t_inpevent *out)
{
	const t_key	*key;
	int			n;

	key = layout_key(k->layout, raw->code);
	if (!key || kbd_ghost_release(k, raw))
		return (0);
	kbd_mod_apply(k, raw);
	key_event(k, key, raw, &out[0]);
	n = 1;
	if (!raw->release)
		n += char_events(k, key, raw, &out[1]);
	return (n);
}
