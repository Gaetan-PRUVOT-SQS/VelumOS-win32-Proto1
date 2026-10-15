#include "dr_int.h"

static uint32_t	dr_cp(t_dstr16 s, uint32_t *i)
{
	uint32_t	c;

	c = s.p[(*i)++];
	if (c >= 0xd800 && c <= 0xdbff && *i < s.n && s.p[*i] >= 0xdc00
		&& s.p[*i] <= 0xdfff)
		return (0x10000 + ((c - 0xd800) << 10) + (s.p[(*i)++] - 0xdc00));
	if (c >= 0xd800 && c <= 0xdfff)
		return (0xfffd);
	return (c);
}

static uint32_t	dr_enc3(uint32_t c, uint8_t *b)
{
	if (c < 0x10000)
	{
		b[0] = (uint8_t)(0xe0 | (c >> 12));
		b[1] = (uint8_t)(0x80 | ((c >> 6) & 0x3f));
		b[2] = (uint8_t)(0x80 | (c & 0x3f));
		return (3);
	}
	b[0] = (uint8_t)(0xf0 | (c >> 18));
	b[1] = (uint8_t)(0x80 | ((c >> 12) & 0x3f));
	b[2] = (uint8_t)(0x80 | ((c >> 6) & 0x3f));
	b[3] = (uint8_t)(0x80 | (c & 0x3f));
	return (4);
}

static uint32_t	dr_enc(uint32_t c, uint8_t *b)
{
	if (c < 0x80)
	{
		b[0] = (uint8_t)c;
		return (1);
	}
	if (c < 0x800)
	{
		b[0] = (uint8_t)(0xc0 | (c >> 6));
		b[1] = (uint8_t)(0x80 | (c & 0x3f));
		return (2);
	}
	return (dr_enc3(c, b));
}

size_t	dr_utf8(t_dstr16 s, char *out, size_t cap)
{
	uint8_t		b[4];
	uint32_t	i;
	uint32_t	k;
	size_t		n;

	i = 0;
	n = 0;
	if (cap == 0)
		return (0);
	while (i < s.n)
	{
		k = dr_enc(dr_cp(s, &i), b);
		if (n + k >= cap)
			break ;
		memcpy(out + n, b, k);
		n += k;
	}
	out[n] = '\0';
	return (n);
}

uint32_t	dr_dec16(int64_t v, uint16_t *out)
{
	uint16_t	tmp[20];
	uint64_t	u;
	uint32_t	n;
	uint32_t	k;

	u = (uint64_t)v;
	if (v < 0)
		u = 0 - u;
	n = 0;
	while (u != 0 || n == 0)
	{
		tmp[n++] = (uint16_t)('0' + u % 10);
		u /= 10;
	}
	k = 0;
	if (v < 0)
		out[k++] = '-';
	while (n > 0)
		out[k++] = tmp[--n];
	return (k);
}
