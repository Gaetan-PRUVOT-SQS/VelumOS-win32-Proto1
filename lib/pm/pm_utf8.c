#include "pm_int.h"

static int	seq_len(uint8_t c)
{
	if (c < 0x80)
		return (1);
	if (c >= 0xc2 && c <= 0xdf)
		return (2);
	if (c >= 0xe0 && c <= 0xef)
		return (3);
	if (c >= 0xf0 && c <= 0xf4)
		return (4);
	return (0);
}

static int	second_ok(uint8_t lead, uint8_t c)
{
	if (lead == 0xe0)
		return (c >= 0xa0 && c <= 0xbf);
	if (lead == 0xed)
		return (c >= 0x80 && c <= 0x9f);
	if (lead == 0xf0)
		return (c >= 0x90 && c <= 0xbf);
	if (lead == 0xf4)
		return (c >= 0x80 && c <= 0x8f);
	return (c >= 0x80 && c <= 0xbf);
}

int	pm_utf8_ok(const char *s, size_t n)
{
	size_t	i;
	size_t	k;
	size_t	len;

	i = 0;
	while (i < n)
	{
		len = (size_t)seq_len((uint8_t)s[i]);
		if (len == 0 || len > n - i)
			return (0);
		if (len > 1 && !second_ok((uint8_t)s[i], (uint8_t)s[i + 1]))
			return (0);
		k = 2;
		while (k < len)
		{
			if (((uint8_t)s[i + k] & 0xc0) != 0x80)
				return (0);
			k++;
		}
		i += len;
	}
	return (1);
}

size_t	pm_nlen(const char *s, size_t max)
{
	size_t	n;

	n = 0;
	while (n < max && s[n])
		n++;
	return (n);
}
