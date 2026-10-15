#include "fake.h"

static int	seq_len(const uint8_t *s)
{
	uint32_t	cp;
	int			n;
	int			i;

	if (s[0] < 0x80)
		return (1);
	n = 0;
	if (s[0] >= 0xc0 && s[0] < 0xe0)
		n = 1;
	else if (s[0] >= 0xe0 && s[0] < 0xf0)
		n = 2;
	else if (s[0] >= 0xf0 && s[0] < 0xf8)
		n = 3;
	if (n == 0)
		return (-1);
	cp = s[0] & (0x3fu >> n);
	i = 1;
	while (i <= n && (s[i] & 0xc0) == 0x80)
		cp = (cp << 6) | (s[i++] & 0x3fu);
	if (i <= n || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
		return (-1);
	if ((n == 1 && cp < 0x80) || (n == 2 && cp < 0x800)
		|| (n == 3 && cp < 0x10000))
		return (-1);
	return (n + 1);
}

bool	fake_utf8_valid(const char *s)
{
	int	n;

	while (*s)
	{
		n = seq_len((const uint8_t *)s);
		if (n < 0)
			return (false);
		s += n;
	}
	return (true);
}

bool	fake_utf8_boundary(const char *s, uint32_t pos)
{
	uint32_t	p;
	int			n;

	p = 0;
	while (p < pos && s[p])
	{
		n = seq_len((const uint8_t *)s + p);
		if (n < 0)
			n = 1;
		p += (uint32_t)n;
	}
	return (p == pos);
}

uint32_t	fake_utf8_chars(const char *s)
{
	uint32_t	count;
	int			n;

	count = 0;
	while (*s)
	{
		n = seq_len((const uint8_t *)s);
		if (n < 0)
			n = 1;
		s += n;
		count++;
	}
	return (count);
}
