#include "rt_int.h"

static int	rt_utf8_cp(const uint8_t **p, uint32_t *cp)
{
	const uint8_t	*s;
	uint32_t		n;
	uint32_t		i;
	uint32_t		low;

	s = *p;
	if (s[0] >= 0xf8 || (s[0] & 0xc0) == 0x80)
		return (E_INVAL);
	n = (s[0] >= 0xc0) + (s[0] >= 0xe0) + (s[0] >= 0xf0);
	*cp = s[0] & (0xffu >> (n + 1 + (n > 0)));
	i = 1;
	while (i <= n)
	{
		if ((s[i] & 0xc0) != 0x80)
			return (E_INVAL);
		*cp = (*cp << 6) | (s[i++] & 0x3fu);
	}
	low = 0;
	if (n > 0)
		low = 0x80u << (4 * (n - 1) + (n == 3));
	if (*cp < low || *cp > 0x10ffff || (*cp >= 0xd800 && *cp < 0xe000))
		return (E_INVAL);
	*p = s + n + 1;
	return (0);
}

int	rt_utf8_to16(const char *utf8, uint16_t *out)
{
	const uint8_t	*p;
	uint32_t		cp;
	uint32_t		n;

	p = (const uint8_t *)utf8;
	n = 0;
	while (*p)
	{
		if (rt_utf8_cp(&p, &cp) != 0)
			return (E_INVAL);
		if (cp >= 0x10000 && out)
		{
			out[n] = (uint16_t)(0xd800 + ((cp - 0x10000) >> 10));
			out[n + 1] = (uint16_t)(0xdc00 + ((cp - 0x10000) & 0x3ff));
		}
		else if (out)
			out[n] = (uint16_t)cp;
		n += 1 + (cp >= 0x10000);
		if (n > DSTR_MAX)
			return (E_RANGE);
	}
	return ((int)n);
}

static uint32_t	rt_cp16(t_dstr16 s, uint32_t *i)
{
	uint32_t	c;
	uint32_t	d;

	c = s.p[(*i)++];
	if (c < 0xd800 || c >= 0xe000)
		return (c);
	if (c >= 0xdc00 || *i >= s.n)
		return (0xfffd);
	d = s.p[*i];
	if (d < 0xdc00 || d >= 0xe000)
		return (0xfffd);
	(*i)++;
	return (0x10000 + ((c - 0xd800) << 10) + (d - 0xdc00));
}

static uint32_t	rt_put8(uint32_t cp, uint8_t *b)
{
	uint32_t	n;
	uint32_t	i;

	n = 1 + (cp >= 0x80) + (cp >= 0x800) + (cp >= 0x10000);
	if (n == 1)
	{
		b[0] = (uint8_t)cp;
		return (1);
	}
	i = n;
	while (--i > 0)
	{
		b[i] = (uint8_t)(0x80 | (cp & 0x3f));
		cp >>= 6;
	}
	b[0] = (uint8_t)((0xf00u >> n) | cp);
	return (n);
}

int	rt_utf16_to8(t_dstr16 s, t_text out)
{
	uint8_t		b[4];
	uint32_t	i;
	uint32_t	k;
	uint32_t	j;
	size_t		n;

	i = 0;
	n = 0;
	while (i < s.n)
	{
		k = rt_put8(rt_cp16(s, &i), b);
		if (!out.p || n + k >= out.cap)
			return (E_OVERFLOW);
		j = 0;
		while (j < k)
			out.p[n++] = (char)b[j++];
	}
	if (!out.p || n >= out.cap)
		return (E_OVERFLOW);
	out.p[n] = '\0';
	return ((int)n);
}
