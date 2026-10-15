#include "fat.h"
#include "velum/err.h"

static int	utf8_cp(const uint8_t *s, uint32_t *cp)
{
	int	n;
	int	i;

	n = 0;
	*cp = s[0];
	if (s[0] >= 0xF0 && s[0] <= 0xF4)
		n = 3;
	else if (s[0] >= 0xE0 && s[0] <= 0xEF)
		n = 2;
	else if (s[0] >= 0xC2 && s[0] <= 0xDF)
		n = 1;
	else if (s[0] >= 0x80)
		return (E_INVAL);
	if (n)
		*cp = s[0] & (0x3F >> n);
	i = 1;
	while (i <= n)
	{
		if ((s[i] & 0xC0) != 0x80)
			return (E_INVAL);
		*cp = (*cp << 6) | (s[i] & 0x3F);
		i++;
	}
	return (n + 1);
}

static int	put_unit(uint32_t cp, uint16_t *u, uint32_t *k, uint32_t max)
{
	if (cp >= 0x10000)
	{
		if (*k + 2 > max)
			return (E_RANGE);
		cp -= 0x10000;
		u[(*k)++] = (uint16_t)(0xD800 | (cp >> 10));
		u[(*k)++] = (uint16_t)(0xDC00 | (cp & 0x3FF));
		return (0);
	}
	if ((cp >= 0xD800 && cp <= 0xDFFF) || *k + 1 > max)
		return (E_RANGE);
	u[(*k)++] = (uint16_t)cp;
	return (0);
}

int	fat_utf8_to16(const char *s, uint16_t *u, uint32_t max)
{
	const uint8_t	*p;
	uint32_t		cp;
	uint32_t		k;
	int				n;

	p = (const uint8_t *)s;
	k = 0;
	while (*p)
	{
		n = utf8_cp(p, &cp);
		if (n < 0)
			return (E_INVAL);
		if (put_unit(cp, u, &k, max) < 0)
			return (E_RANGE);
		p += n;
	}
	return ((int)k);
}
