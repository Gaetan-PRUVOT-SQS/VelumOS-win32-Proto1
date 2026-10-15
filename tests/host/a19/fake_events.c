#include <string.h>
#include "fake.h"

bool	fake_press(t_ctlroot *r, uint32_t code, uint32_t mods)
{
	t_inpevent	ev;

	memset(&ev, 0, sizeof(ev));
	ev.type = INP_KEY_DOWN;
	ev.code = code;
	ev.mods = mods;
	return (ctl_key(r, &ev));
}

bool	fake_char(t_ctlroot *r, uint32_t cp)
{
	t_inpevent	ev;

	memset(&ev, 0, sizeof(ev));
	ev.type = INP_CHAR;
	ev.code = cp;
	return (ctl_key(r, &ev));
}

static uint32_t	decode(const char *s, int *len)
{
	uint8_t	b;

	b = (uint8_t)s[0];
	*len = 1;
	if (b < 0x80)
		return (b);
	if (b < 0xe0)
		*len = 2;
	else if (b < 0xf0)
		*len = 3;
	else
		*len = 4;
	if (*len == 2)
		return (((b & 0x1fu) << 6) | (s[1] & 0x3fu));
	if (*len == 3)
		return (((b & 0x0fu) << 12) | ((s[1] & 0x3fu) << 6) | (s[2] & 0x3fu));
	return (((b & 0x07u) << 18) | ((s[1] & 0x3fu) << 12)
		| ((s[2] & 0x3fu) << 6) | (s[3] & 0x3fu));
}

int	fake_type(t_ctlroot *r, const char *utf8)
{
	int	n;
	int	len;

	n = 0;
	while (*utf8)
	{
		fake_char(r, decode(utf8, &len));
		utf8 += len;
		n++;
	}
	return (n);
}
