#include "stdlib.h"
#include "string.h"

char	*strdup(const char *s)
{
	size_t	len;
	char	*copy;

	len = strlen(s);
	copy = malloc(len + 1);
	if (!copy)
		return (NULL);
	memcpy(copy, s, len + 1);
	return (copy);
}

char	*strndup(const char *s, size_t max)
{
	size_t	len;
	char	*copy;

	len = strnlen(s, max);
	copy = malloc(len + 1);
	if (!copy)
		return (NULL);
	memcpy(copy, s, len);
	copy[len] = '\0';
	return (copy);
}

void	explicit_bzero(void *s, size_t n)
{
	volatile unsigned char	*p;

	p = s;
	while (n--)
	{
		*p = 0;
		p++;
	}
}
