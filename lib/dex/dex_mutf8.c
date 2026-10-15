#include "dex_int.h"

static int	cont(const uint8_t *p, size_t i, size_t max)
{
	return (i < max && (p[i] & 0xc0) == 0x80);
}

static size_t	seq_len(const uint8_t *p, size_t i, size_t max)
{
	uint32_t	v;

	if (p[i] < 0x80)
		return (1);
	if ((p[i] & 0xe0) == 0xc0 && cont(p, i + 1, max))
	{
		v = ((uint32_t)(p[i] & 0x1f) << 6) | (p[i + 1] & 0x3f);
		if (v >= 0x80 || v == 0)
			return (2);
		return (0);
	}
	if ((p[i] & 0xf0) == 0xe0 && cont(p, i + 1, max) && cont(p, i + 2, max))
	{
		v = ((uint32_t)(p[i] & 0x0f) << 12) | ((p[i + 1] & 0x3fu) << 6);
		if (v >= 0x800)
			return (3);
	}
	return (0);
}

int	dex_mutf8_check(t_span s, uint32_t *size, uint32_t *units)
{
	size_t		i;
	size_t		n;
	uint32_t	u;

	i = 0;
	u = 0;
	while (i < s.len && s.p[i] != 0)
	{
		n = seq_len(s.p, i, s.len);
		if (n == 0)
			return (E_INVAL);
		i += n;
		u++;
	}
	if (i >= s.len || i > 0x7fffffff)
		return (E_INVAL);
	*size = (uint32_t)i;
	*units = u;
	return (E_OK);
}
