#include "ctl_int.h"

static uint32_t	enc2(uint32_t cp, char *out)
{
	out[0] = (char)(0xc0 | (cp >> 6));
	out[1] = (char)(0x80 | (cp & 0x3f));
	return (2);
}

static uint32_t	enc3(uint32_t cp, char *out)
{
	if (cp >= 0xd800 && cp <= 0xdfff)
		return (0);
	out[0] = (char)(0xe0 | (cp >> 12));
	out[1] = (char)(0x80 | ((cp >> 6) & 0x3f));
	out[2] = (char)(0x80 | (cp & 0x3f));
	return (3);
}

static uint32_t	enc4(uint32_t cp, char *out)
{
	if (cp > 0x10ffff)
		return (0);
	out[0] = (char)(0xf0 | (cp >> 18));
	out[1] = (char)(0x80 | ((cp >> 12) & 0x3f));
	out[2] = (char)(0x80 | ((cp >> 6) & 0x3f));
	out[3] = (char)(0x80 | (cp & 0x3f));
	return (4);
}

uint32_t	ctl_u8_encode(uint32_t cp, char *out)
{
	if (cp < 0x80)
	{
		out[0] = (char)cp;
		return (1);
	}
	if (cp < 0x800)
		return (enc2(cp, out));
	if (cp < 0x10000)
		return (enc3(cp, out));
	return (enc4(cp, out));
}

uint32_t	ctl_u8_prev(const char *s, uint32_t pos)
{
	uint32_t	p;

	if (pos == 0)
		return (0);
	p = pos - 1;
	while (p > 0 && pos - p < 4 && ((uint8_t)s[p] & 0xc0) == 0x80)
		p--;
	if (p + ctl_u8_len(s + p, pos - p) == pos)
		return (p);
	return (pos - 1);
}
