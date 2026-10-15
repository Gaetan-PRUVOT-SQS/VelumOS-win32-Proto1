#include "vfs_int.h"

static uint32_t	next_cp(const char **s)
{
	const uint8_t	*p;
	uint32_t		cp;
	int				n;

	p = (const uint8_t *)*s;
	n = 0;
	cp = p[0];
	if (p[0] >= 0xF0)
		n = 3;
	else if (p[0] >= 0xE0)
		n = 2;
	else if (p[0] >= 0xC0)
		n = 1;
	if (n)
		cp = p[0] & (0x3F >> n);
	*s += 1;
	while (n-- > 0 && (**s & 0xC0) == 0x80)
	{
		cp = (cp << 6) | (**s & 0x3F);
		*s += 1;
	}
	return (cp);
}

static uint32_t	fold_ext_a(uint32_t c)
{
	if ((c >= 0x100 && c <= 0x137) || (c >= 0x14A && c <= 0x177))
		return (c & ~1u);
	if ((c >= 0x139 && c <= 0x148) || (c >= 0x179 && c <= 0x17E))
	{
		if ((c & 1) == 0)
			return (c - 1);
	}
	return (c);
}

static uint32_t	fold(uint32_t c)
{
	if (c >= 'a' && c <= 'z')
		return (c - 32);
	if (c >= 0xE0 && c <= 0xFE && c != 0xF7)
		return (c - 32);
	if (c == 0xFF)
		return (0x178);
	if (c >= 0x3B1 && c <= 0x3C9 && c != 0x3C2)
		return (c - 32);
	if (c >= 0x430 && c <= 0x44F)
		return (c - 32);
	if (c >= 0x450 && c <= 0x45F)
		return (c - 80);
	return (fold_ext_a(c));
}

int	vname_eq(const char *a, const char *b)
{
	while (*a && *b)
	{
		if (fold(next_cp(&a)) != fold(next_cp(&b)))
			return (0);
	}
	return (*a == *b);
}
