#include <stdio.h>
#include <string.h>
#include "th.h"

int	th_scan(t_scan2 *s, const uint8_t *b, int n, t_keyraw *out)
{
	int			i;
	int			cnt;
	t_keyraw	raw;

	cnt = 0;
	i = 0;
	while (i < n)
	{
		if (scan2_feed(s, b[i], &raw))
		{
			if (out)
				out[cnt] = raw;
			cnt++;
		}
		i++;
	}
	return (cnt);
}

t_keyraw	th_raw(uint16_t code, int release, int repeat)
{
	t_keyraw	r;

	r.code = code;
	r.release = (uint8_t)release;
	r.repeat = (uint8_t)repeat;
	return (r);
}

int	th_press(t_kbd *k, uint16_t code, t_inpevent *out)
{
	t_keyraw	raw;

	raw = th_raw(code, 0, 0);
	return (kbd_translate(k, &raw, out));
}

int	th_release(t_kbd *k, uint16_t code, t_inpevent *out)
{
	t_keyraw	raw;

	raw = th_raw(code, 1, 0);
	return (kbd_translate(k, &raw, out));
}

uint32_t	th_char(t_kbd *k, uint16_t code)
{
	t_inpevent	ev[KEY_OUT_MAX];
	int			n;
	uint32_t	ch;

	n = th_press(k, code, ev);
	ch = 0;
	if (n >= 2)
		ch = ev[1].code;
	th_release(k, code, ev);
	return (ch);
}
