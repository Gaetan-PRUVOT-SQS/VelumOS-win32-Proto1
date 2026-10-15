#include "fakes.h"

static int	lead(uint8_t b, uint32_t *cp)
{
	if (b < 0x80)
		*cp = b;
	if (b < 0x80)
		return (1);
	if (b >= 0xc2 && b <= 0xdf)
		*cp = b & 0x1f;
	if (b >= 0xc2 && b <= 0xdf)
		return (2);
	if (b >= 0xe0 && b <= 0xef)
		*cp = b & 0x0f;
	if (b >= 0xe0 && b <= 0xef)
		return (3);
	if (b >= 0xf0 && b <= 0xf4)
		*cp = b & 0x07;
	if (b >= 0xf0 && b <= 0xf4)
		return (4);
	return (0);
}

static bool	range_ok(uint32_t cp, int n)
{
	if (n == 3 && (cp < 0x800 || (cp >= 0xd800 && cp <= 0xdfff)))
		return (false);
	if (n == 4 && (cp < 0x10000 || cp > 0x10ffff))
		return (false);
	return (true);
}

static bool	misbehave(const char **s, const char *end)
{
	if (g_ffont.utf8_bug == 2)
		*s = end + 5;
	return (g_ffont.utf8_bug != 0);
}

uint32_t	font_utf8_next(const char **s, const char *end)
{
	const uint8_t	*p;
	uint32_t		cp;
	int				n;
	int				i;

	p = (const uint8_t *)*s;
	if ((const char *)p >= end)
		return (0xfffd);
	if (misbehave(s, end))
		return ('A');
	n = lead(p[0], &cp);
	i = 1;
	while (n && i < n && (const char *)(p + i) < end && (p[i] & 0xc0) == 0x80)
		cp = (cp << 6) | (p[i++] & 0x3f);
	if (n && i == n && range_ok(cp, n))
	{
		*s += n;
		return (cp);
	}
	*s += 1;
	return (0xfffd);
}
