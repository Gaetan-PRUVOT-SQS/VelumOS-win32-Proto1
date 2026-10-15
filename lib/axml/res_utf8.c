#include "axml_int.h"

static uint32_t	lead_need(uint8_t b)
{
	if (b >= 0xc2 && b <= 0xdf)
		return (1);
	if (b >= 0xe0 && b <= 0xef)
		return (2);
	if (b >= 0xf0 && b <= 0xf4)
		return (3);
	return (0);
}

static int	second_ok(uint8_t lead, uint8_t b)
{
	if (lead == 0xe0)
		return (b >= 0xa0 && b <= 0xbf);
	if (lead == 0xed)
		return (b >= 0x80 && b <= 0x9f);
	if (lead == 0xf0)
		return (b >= 0x90 && b <= 0xbf);
	if (lead == 0xf4)
		return (b >= 0x80 && b <= 0x8f);
	return ((b & 0xc0) == 0x80);
}

uint32_t	res_u8_seq(const uint8_t *s, uint32_t avail)
{
	uint32_t	need;
	uint32_t	i;

	if (avail == 0 || s[0] == 0)
		return (0);
	if (s[0] < 0x80)
		return (1);
	need = lead_need(s[0]);
	if (need == 0 || need >= avail || !second_ok(s[0], s[1]))
		return (0);
	i = 2;
	while (i <= need)
	{
		if ((s[i] & 0xc0) != 0x80)
			return (0);
		i++;
	}
	return (need + 1);
}

static uint32_t	put_wide(uint32_t cp, char *d)
{
	if (cp < 0x10000)
	{
		d[0] = (char)(0xe0 | (cp >> 12));
		d[1] = (char)(0x80 | ((cp >> 6) & 0x3f));
		d[2] = (char)(0x80 | (cp & 0x3f));
		return (3);
	}
	d[0] = (char)(0xf0 | (cp >> 18));
	d[1] = (char)(0x80 | ((cp >> 12) & 0x3f));
	d[2] = (char)(0x80 | ((cp >> 6) & 0x3f));
	d[3] = (char)(0x80 | (cp & 0x3f));
	return (4);
}

uint32_t	res_u8_put(uint32_t cp, t_text out, size_t w)
{
	uint32_t	n;

	n = 1 + (cp >= 0x80) + (cp >= 0x800) + (cp >= 0x10000);
	if (w >= out.cap || out.cap - w <= n)
		return (0);
	if (n == 1)
		out.p[w] = (char)cp;
	if (n == 2)
	{
		out.p[w] = (char)(0xc0 | (cp >> 6));
		out.p[w + 1] = (char)(0x80 | (cp & 0x3f));
	}
	if (n > 2)
		put_wide(cp, out.p + w);
	return (n);
}
