#include <stddef.h>
#include "velum/libk.h"

size_t	strlcpy(char *dst, const char *src, size_t size)
{
	size_t	len;
	size_t	n;

	len = strlen(src);
	if (!size)
		return (len);
	n = len;
	if (n >= size)
		n = size - 1;
	memcpy(dst, src, n);
	dst[n] = '\0';
	return (len);
}

size_t	strlcat(char *dst, const char *src, size_t size)
{
	size_t	dlen;

	dlen = strnlen(dst, size);
	if (dlen == size)
		return (size + strlen(src));
	return (dlen + strlcpy(dst + dlen, src, size - dlen));
}

char	*strrchr(const char *s, int c)
{
	const char	*last;

	last = NULL;
	while (*s)
	{
		if (*s == (char)c)
			last = s;
		s++;
	}
	if (c == 0)
		return ((char *)s);
	return ((char *)last);
}

char	*strstr(const char *hay, const char *needle)
{
	size_t	n;

	n = strlen(needle);
	if (!n)
		return ((char *)hay);
	while (*hay)
	{
		if (!strncmp(hay, needle, n))
			return ((char *)hay);
		hay++;
	}
	return (NULL);
}
