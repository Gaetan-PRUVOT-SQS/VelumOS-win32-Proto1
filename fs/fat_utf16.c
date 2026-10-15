#include "fat.h"
#include "velum/err.h"

static uint32_t	utf8_len(uint32_t cp)
{
	if (cp < 0x80)
		return (1);
	if (cp < 0x800)
		return (2);
	if (cp < 0x10000)
		return (3);
	return (4);
}

static void	utf8_put(uint32_t cp, char *out, uint32_t len)
{
	uint32_t	i;

	if (len == 1)
	{
		out[0] = (char)cp;
		return ;
	}
	i = len;
	while (--i > 0)
	{
		out[i] = (char)(0x80 | (cp & 0x3F));
		cp >>= 6;
	}
	out[0] = (char)((0xF00u >> len) | cp);
}

static int	next_cp(const uint16_t *u, uint32_t n, uint32_t *i, uint32_t *cp)
{
	*cp = u[(*i)++];
	if (*cp >= 0xD800 && *cp <= 0xDBFF)
	{
		if (*i >= n || u[*i] < 0xDC00 || u[*i] > 0xDFFF)
			return (E_INVAL);
		*cp = 0x10000 + ((*cp - 0xD800) << 10) + (u[(*i)++] - 0xDC00);
	}
	else if (*cp >= 0xDC00 && *cp <= 0xDFFF)
		return (E_INVAL);
	if (*cp < 0x20 || *cp == 0x7F || *cp == '/' || *cp == '\\'
		|| (*cp >= 0x80 && *cp <= 0x9F))
		return (E_INVAL);
	return (0);
}

int	fat_utf16_to8(const uint16_t *u, uint32_t n, char *out, uint32_t cap)
{
	uint32_t	i;
	uint32_t	o;
	uint32_t	cp;
	uint32_t	len;

	i = 0;
	o = 0;
	while (i < n)
	{
		if (next_cp(u, n, &i, &cp) < 0)
			return (E_INVAL);
		len = utf8_len(cp);
		if (o + len >= cap)
			return (E_RANGE);
		utf8_put(cp, out + o, len);
		o += len;
	}
	out[o] = '\0';
	return ((int)o);
}
