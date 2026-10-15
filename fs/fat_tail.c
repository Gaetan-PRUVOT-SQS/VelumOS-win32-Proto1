#include "fat.h"
#include "velum/libk.h"

static void	put_tail(uint8_t *raw, uint32_t k)
{
	char		digits[8];
	uint32_t	nd;
	uint32_t	keep;

	nd = 0;
	while (k || nd == 0)
	{
		digits[nd++] = (char)('0' + k % 10);
		k /= 10;
	}
	keep = 0;
	while (keep < 8 && raw[keep] != ' ')
		keep++;
	if (keep > 7 - nd)
		keep = 7 - nd;
	raw[keep++] = '~';
	while (nd)
		raw[keep++] = (uint8_t)digits[--nd];
	while (keep < 8)
		raw[keep++] = ' ';
}

static uint32_t	name_hash(const char *s)
{
	uint32_t	h;

	h = 2166136261u;
	while (*s)
	{
		h ^= (uint8_t)s[0];
		s++;
		h *= 16777619u;
	}
	return (h);
}

void	fat_sname_try(const char *name, const uint8_t *basis, uint32_t k,
		uint8_t *out)
{
	uint32_t	h;
	uint32_t	i;

	memcpy(out, basis, 11);
	if (k <= 4)
	{
		put_tail(out, k);
		return ;
	}
	if (out[1] == ' ')
		out[1] = '_';
	h = name_hash(name) + k;
	i = 0;
	while (i < 4)
	{
		out[2 + i] = (uint8_t)"0123456789ABCDEF"[(h >> (12 - 4 * i)) & 15];
		i++;
	}
	out[6] = ' ';
	out[7] = ' ';
	put_tail(out, 1);
}
