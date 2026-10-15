#include "fake_font.h"

static uint32_t	fk_tail(const char **s, const char *end, int32_t more,
					uint32_t cp)
{
	while (more > 0)
	{
		if (*s >= end || ((unsigned char)(**s) & 0xc0) != 0x80)
			return (0xfffd);
		cp = (cp << 6) | ((unsigned char)(**s) & 0x3f);
		(*s)++;
		more--;
	}
	return (cp);
}

uint32_t	fk_utf8(const char **s, const char *end)
{
	unsigned char	c;

	c = (unsigned char)(**s);
	(*s)++;
	if (c < 0x80)
		return (c);
	if ((c & 0xe0) == 0xc0)
		return (fk_tail(s, end, 1, c & 0x1f));
	if ((c & 0xf0) == 0xe0)
		return (fk_tail(s, end, 2, c & 0x0f));
	return (0xfffd);
}

int32_t	fk_len(const char *s)
{
	int32_t	n;

	n = 0;
	while (s[n] != '\0')
		n++;
	return (n);
}
