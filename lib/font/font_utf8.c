#include "font_int.h"

static int32_t	lead_need(uint8_t lead)
{
	if (lead >= 0xC2 && lead <= 0xDF)
		return (1);
	if (lead >= 0xE0 && lead <= 0xEF)
		return (2);
	if (lead >= 0xF0 && lead <= 0xF4)
		return (3);
	return (0);
}

static int	continuation_ok(const uint8_t *p, int32_t i)
{
	uint8_t	low;
	uint8_t	high;

	low = 0x80;
	high = 0xBF;
	if (i == 1 && p[0] == 0xE0)
		low = 0xA0;
	if (i == 1 && p[0] == 0xF0)
		low = 0x90;
	if (i == 1 && p[0] == 0xED)
		high = 0x9F;
	if (i == 1 && p[0] == 0xF4)
		high = 0x8F;
	return (p[i] >= low && p[i] <= high);
}

static int32_t	valid_tail(const uint8_t *p, ptrdiff_t avail, int32_t need)
{
	int32_t	n;

	n = 0;
	while (n < need && n + 1 < avail && continuation_ok(p, n + 1))
		n++;
	return (n);
}

static uint32_t	decode_sequence(const uint8_t *p, int32_t need)
{
	uint32_t	cp;
	int32_t		i;

	cp = p[0] & (0x3F >> need);
	i = 1;
	while (i <= need)
	{
		cp = (cp << 6) | (p[i] & 0x3F);
		i++;
	}
	return (cp);
}

uint32_t	font_utf8_next(const char **s, const char *end)
{
	const uint8_t	*p;
	int32_t			need;
	int32_t			got;

	if (!s || !*s || *s >= end)
		return (0);
	p = (const uint8_t *)*s;
	if (p[0] < 0x80)
	{
		*s += 1;
		return (p[0]);
	}
	need = lead_need(p[0]);
	got = valid_tail(p, end - *s, need);
	*s += 1 + got;
	if (!need || got < need)
		return (FONT_REPLACEMENT);
	return (decode_sequence(p, need));
}
