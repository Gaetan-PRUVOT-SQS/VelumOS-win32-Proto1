#include <stddef.h>

size_t	strlen(const char *s)
{
	size_t	n;

	n = 0;
	while (s[n])
		n++;
	return (n);
}

size_t	strnlen(const char *s, size_t max)
{
	size_t	n;

	n = 0;
	while (n < max && s[n])
		n++;
	return (n);
}

int	strcmp(const char *a, const char *b)
{
	while (*a && *a == *b)
	{
		a++;
		b++;
	}
	return ((unsigned char)*a - (unsigned char)*b);
}

int	strncmp(const char *a, const char *b, size_t n)
{
	while (n && *a && *a == *b)
	{
		a++;
		b++;
		n--;
	}
	if (!n)
		return (0);
	return ((unsigned char)*a - (unsigned char)*b);
}

char	*strchr(const char *s, int c)
{
	while (*s)
	{
		if (*s == (char)c)
			return ((char *)s);
		s++;
	}
	if (c == 0)
		return ((char *)s);
	return (NULL);
}
