#include "kbd.h"

static uint32_t	ctrl_char(const t_key *key, uint32_t ch)
{
	if ((key->flags & KF_LETTER) && key->vk >= 'A' && key->vk <= 'Z')
		return ((uint32_t)(key->vk - 'A' + 1));
	if (ch == 0x08 || ch == 0x09 || ch == 0x0d || ch == 0x1b)
		return (ch);
	return (0);
}

static uint32_t	pad_char(const t_kbd *k, const t_key *key)
{
	if (!kbd_numpad(k))
		return (0);
	return (key->ch[COL_NORMAL]);
}

static uint32_t	column_char(const t_kbd *k, const t_key *key, int altgr)
{
	int	shift;
	int	caps;

	shift = (k->held & KM_SHIFT) != 0;
	caps = (k->locks & INPUT_LED_CAPS) != 0;
	if (altgr && shift)
		return (key->ch[COL_SHIFT_ALTGR]);
	if (altgr)
		return (key->ch[COL_ALTGR]);
	if (caps && (key->flags & KF_LETTER))
		shift = !shift;
	if (caps && (key->flags & KF_CAPSCH) && !shift)
		return (key->ch[COL_CAPS]);
	if (shift)
		return (key->ch[COL_SHIFT]);
	return (key->ch[COL_NORMAL]);
}

uint32_t	kbd_resolve(const t_kbd *k, const t_key *key)
{
	int			altgr;
	uint32_t	ch;

	altgr = kbd_altgr(k);
	if (!altgr && (k->held & (KM_ALT | KM_WIN)))
		return (0);
	if (key->flags & KF_PAD)
		return (pad_char(k, key));
	if (key->flags & KF_FIXED)
		ch = key->ch[COL_NORMAL];
	else
		ch = column_char(k, key, altgr);
	if (!altgr && (k->held & KM_CTRL))
		return (ctrl_char(key, ch));
	return (ch);
}
