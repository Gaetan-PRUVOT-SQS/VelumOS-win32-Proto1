#include "fat.h"
#include "velum/libk.h"

static int	sname_legal(uint8_t c)
{
	if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		return (1);
	return (strchr("$%'-_@~`!(){}^#&", c) != NULL);
}

static char	map_char(const char *q, uint32_t *adv, int *lossy)
{
	uint8_t	c;

	c = (uint8_t)q[0];
	*adv = 1;
	if (c >= 0x80)
	{
		while (((uint8_t)q[*adv] & 0xC0) == 0x80)
			(*adv)++;
		*lossy = 1;
		return ('_');
	}
	if (c == ' ')
		return (' ');
	if (c >= 'a' && c <= 'z')
		return ((char)(c - 'a' + 'A'));
	if (sname_legal(c))
		return ((char)c);
	*lossy = 1;
	return ('_');
}

static void	copy_part(const char *q, uint8_t *dst, uint32_t max, int *lossy)
{
	uint32_t	i;
	uint32_t	adv;
	char		c;

	i = 0;
	while (*q && *q != '.' && i < max)
	{
		c = map_char(q, &adv, lossy);
		if (c != ' ')
			dst[i++] = (uint8_t)c;
		q += adv;
	}
}

void	fat_basis(const char *name, uint8_t *raw, int *lossy)
{
	const char	*p;
	const char	*last;

	memset(raw, ' ', 11);
	*lossy = 0;
	p = name;
	while (*p == '.' || *p == ' ')
		p++;
	last = strrchr(p, '.');
	copy_part(p, raw, 8, lossy);
	if (last)
		copy_part(last + 1, raw + 8, 3, lossy);
	if (raw[0] == ' ')
	{
		raw[0] = '_';
		*lossy = 1;
	}
}

int	fat_fits_83(const char *name)
{
	const char	*dot;
	size_t		n;

	n = strlen(name);
	dot = strchr(name, '.');
	if (n == 0 || strchr(name, ' '))
		return (0);
	if (dot == NULL)
		return (n <= 8);
	if (dot == name || strchr(dot + 1, '.'))
		return (0);
	return ((size_t)(dot - name) <= 8 && n - (size_t)(dot - name) - 1 <= 3);
}
