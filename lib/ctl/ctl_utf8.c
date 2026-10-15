#include "ctl_int.h"

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

static bool	second_ok(uint8_t lead, uint8_t b)
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

uint32_t	ctl_u8_len(const char *s, uint32_t avail)
{
	uint32_t	need;
	uint32_t	i;

	if (avail == 0)
		return (0);
	if ((uint8_t)s[0] < 0x80)
		return (1);
	need = lead_need((uint8_t)s[0]);
	if (need == 0 || need >= avail || !second_ok((uint8_t)s[0], (uint8_t)s[1]))
		return (1);
	i = 2;
	while (i <= need)
	{
		if (((uint8_t)s[i] & 0xc0) != 0x80)
			return (1);
		i++;
	}
	return (need + 1);
}

uint32_t	ctl_u8_next(const char *s, uint32_t len, uint32_t pos)
{
	if (pos >= len)
		return (len);
	return (pos + ctl_u8_len(s + pos, len - pos));
}

uint32_t	ctl_u8_count(const char *s, uint32_t len)
{
	uint32_t	pos;
	uint32_t	n;

	pos = 0;
	n = 0;
	while (pos < len)
	{
		pos = ctl_u8_next(s, len, pos);
		n++;
	}
	return (n);
}
