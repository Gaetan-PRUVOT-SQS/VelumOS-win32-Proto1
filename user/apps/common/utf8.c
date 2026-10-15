#include "velum/libk.h"
#include "utf8.h"

static size_t	lead_info(uint8_t b, uint8_t *lo, uint8_t *hi)
{
	*lo = 0x80;
	*hi = 0xbf;
	if (b >= 0xc2 && b <= 0xdf)
		return (2);
	if (b >= 0xe0 && b <= 0xef)
	{
		if (b == 0xe0)
			*lo = 0xa0;
		if (b == 0xed)
			*hi = 0x9f;
		return (3);
	}
	if (b >= 0xf0 && b <= 0xf4)
	{
		if (b == 0xf0)
			*lo = 0x90;
		if (b == 0xf4)
			*hi = 0x8f;
		return (4);
	}
	return (0);
}

size_t	utf8_seq_len(const uint8_t *s, size_t left)
{
	uint8_t	lo;
	uint8_t	hi;
	size_t	n;
	size_t	i;

	if (left == 0)
		return (0);
	if (s[0] < 0x80)
		return (1);
	n = lead_info(s[0], &lo, &hi);
	if (n == 0 || left < n || s[1] < lo || s[1] > hi)
		return (0);
	i = 2;
	while (i < n)
	{
		if (s[i] < 0x80 || s[i] > 0xbf)
			return (0);
		i++;
	}
	return (n);
}

int	utf8_valid(const char *s, size_t len)
{
	size_t	pos;
	size_t	n;

	pos = 0;
	while (pos < len)
	{
		n = utf8_seq_len((const uint8_t *)s + pos, len - pos);
		if (n == 0)
			return (0);
		pos += n;
	}
	return (1);
}

static size_t	unit_len(const char *s, size_t left, int *bad)
{
	size_t	n;

	n = utf8_seq_len((const uint8_t *)s, left);
	*bad = 0;
	if (n == 0 || (n == 1 && ((uint8_t)s[0] < 0x20 || s[0] == 0x7f)))
	{
		*bad = 1;
		return (1);
	}
	return (n);
}

size_t	utf8_clean_copy(char *dst, size_t size, const char *src, size_t len)
{
	size_t	pos;
	size_t	out;
	size_t	n;
	int		bad;

	if (size == 0)
		return (0);
	pos = 0;
	out = 0;
	while (pos < len)
	{
		n = unit_len(src + pos, len - pos, &bad);
		if (out + n + 1 > size)
			break ;
		if (bad)
			dst[out] = '?';
		else
			memcpy(dst + out, src + pos, n);
		pos += n;
		out += n;
	}
	dst[out] = '\0';
	return (out);
}
