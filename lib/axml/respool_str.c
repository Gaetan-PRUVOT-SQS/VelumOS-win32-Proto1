#include "axml_int.h"

static int	len8(const t_respool *p, uint32_t *off, uint32_t *len)
{
	uint32_t	b;

	if (*off >= p->size)
		return (E_INVAL);
	b = p->p[(*off)++];
	*len = b;
	if (!(b & 0x80))
		return (E_OK);
	if (*off >= p->size)
		return (E_INVAL);
	*len = ((b & 0x7f) << 8) | p->p[(*off)++];
	return (E_OK);
}

int	respool_u8(const t_respool *p, uint32_t off, t_text out)
{
	uint32_t	n;
	uint32_t	i;
	uint32_t	k;

	if (len8(p, &off, &n) < 0 || len8(p, &off, &n) < 0)
		return (E_INVAL);
	if (off >= p->size || n >= p->size - off || p->p[off + n] != 0)
		return (E_INVAL);
	if ((size_t)n + 1 > out.cap)
		return (E_OVERFLOW);
	i = 0;
	while (i < n)
	{
		k = res_u8_seq(p->p + off + i, n - i);
		if (k == 0)
			return (E_INVAL);
		while (k--)
		{
			out.p[i] = (char)p->p[off + i];
			i++;
		}
	}
	out.p[n] = '\0';
	return ((int)n);
}

static int	len16(const t_respool *p, uint32_t *off, uint32_t *len)
{
	if (*off > p->size || p->size - *off < 2)
		return (E_INVAL);
	*len = res_rd16(p->p + *off);
	*off += 2;
	if (!(*len & 0x8000))
		return (E_OK);
	if (p->size - *off < 2)
		return (E_INVAL);
	*len = ((*len & 0x7fff) << 16) | res_rd16(p->p + *off);
	*off += 2;
	return (E_OK);
}

static int	codepoint(const uint8_t *s, uint32_t left, uint32_t *cp)
{
	uint32_t	lo;

	*cp = res_rd16(s);
	if (*cp == 0 || (*cp >= 0xdc00 && *cp <= 0xdfff))
		return (E_INVAL);
	if (*cp < 0xd800 || *cp > 0xdbff)
		return (1);
	if (left < 2)
		return (E_INVAL);
	lo = res_rd16(s + 2);
	if (lo < 0xdc00 || lo > 0xdfff)
		return (E_INVAL);
	*cp = 0x10000 + ((*cp - 0xd800) << 10) + (lo - 0xdc00);
	return (2);
}

int	respool_u16(const t_respool *p, uint32_t off, t_text out)
{
	uint32_t	n;
	uint32_t	i;
	uint32_t	cp;
	size_t		w;
	int			k;

	if (len16(p, &off, &n) < 0 || n >= (p->size - off) / 2)
		return (E_INVAL);
	if (res_rd16(p->p + off + 2 * (size_t)n) != 0)
		return (E_INVAL);
	i = 0;
	w = 0;
	while (i < n)
	{
		k = codepoint(p->p + off + 2 * (size_t)i, n - i, &cp);
		if (k < 0)
			return (E_INVAL);
		cp = res_u8_put(cp, out, w);
		if (cp == 0)
			return (E_OVERFLOW);
		w += cp;
		i += (uint32_t)k;
	}
	out.p[w] = '\0';
	return ((int)w);
}
