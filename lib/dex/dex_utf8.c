#include "dex_int.h"

static uint32_t	unit(const uint8_t *p, size_t *i)
{
	uint32_t	v;

	if (p[*i] < 0x80)
		return (p[(*i)++]);
	if ((p[*i] & 0xe0) == 0xc0 && (p[*i + 1] & 0xc0) == 0x80)
	{
		v = ((uint32_t)(p[*i] & 0x1f) << 6) | (p[*i + 1] & 0x3f);
		*i += 2;
		if (v != 0 && v < 0x80)
			return (DEX_NO_INDEX);
		return (v);
	}
	if ((p[*i] & 0xf0) == 0xe0 && (p[*i + 1] & 0xc0) == 0x80
		&& (p[*i + 2] & 0xc0) == 0x80)
	{
		v = ((uint32_t)(p[*i] & 0x0f) << 12) | ((p[*i + 1] & 0x3fu) << 6);
		v |= p[*i + 2] & 0x3f;
		*i += 3;
		if (v >= 0x800)
			return (v);
	}
	return (DEX_NO_INDEX);
}

static uint32_t	point(const uint8_t *p, size_t *i)
{
	uint32_t	hi;
	uint32_t	lo;

	hi = unit(p, i);
	if (hi < 0xd800 || hi > 0xdfff)
		return (hi);
	if (hi > 0xdbff)
		return (DEX_NO_INDEX);
	lo = unit(p, i);
	if (lo < 0xdc00 || lo > 0xdfff)
		return (DEX_NO_INDEX);
	return (0x10000 + ((hi - 0xd800) << 10) + (lo - 0xdc00));
}

static size_t	put(uint32_t cp, char *o)
{
	if (cp < 0x80)
	{
		o[0] = (char)cp;
		return (1);
	}
	if (cp < 0x800)
	{
		o[0] = (char)(0xc0 | (cp >> 6));
		o[1] = (char)(0x80 | (cp & 0x3f));
		return (2);
	}
	if (cp < 0x10000)
	{
		o[0] = (char)(0xe0 | (cp >> 12));
		o[1] = (char)(0x80 | ((cp >> 6) & 0x3f));
		o[2] = (char)(0x80 | (cp & 0x3f));
		return (3);
	}
	o[0] = (char)(0xf0 | (cp >> 18));
	o[1] = (char)(0x80 | ((cp >> 12) & 0x3f));
	o[2] = (char)(0x80 | ((cp >> 6) & 0x3f));
	o[3] = (char)(0x80 | (cp & 0x3f));
	return (4);
}

static int	emit(t_text out, size_t *o, uint32_t cp)
{
	char	tmp[4];
	size_t	n;
	size_t	k;

	n = put(cp, tmp);
	if (n >= out.cap - *o)
		return (E_OVERFLOW);
	k = 0;
	while (k < n)
	{
		out.p[*o + k] = tmp[k];
		k++;
	}
	*o += n;
	out.p[*o] = 0;
	return (E_OK);
}

int	mutf8_to_utf8(const char *in, t_text out)
{
	size_t		i;
	size_t		o;
	uint32_t	cp;

	if (!in || !out.p || out.cap == 0 || out.cap > 0x7fffffff)
		return (E_INVAL);
	i = 0;
	o = 0;
	out.p[0] = 0;
	while (in[i] != 0)
	{
		cp = point((const uint8_t *)in, &i);
		if (cp == DEX_NO_INDEX)
			return (E_INVAL);
		if (cp == 0)
			return (E_NOTSUP);
		if (emit(out, &o, cp) != E_OK)
			return (E_OVERFLOW);
	}
	return ((int)o);
}
