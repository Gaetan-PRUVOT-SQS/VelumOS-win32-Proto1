#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

static uint32_t	lfn_off(uint32_t j)
{
	if (j < 5)
		return (1 + 2 * j);
	if (j < 11)
		return (14 + 2 * (j - 5));
	return (28 + 2 * (j - 11));
}

static void	lfn_store(t_flfn *l, const uint8_t *e, uint32_t ord)
{
	uint32_t	base;
	uint32_t	j;

	base = (ord - 1) * FAT_LFN_CHARS;
	j = 0;
	while (j < FAT_LFN_CHARS)
	{
		l->u[base + j] = (uint16_t)le16(e + lfn_off(j));
		j++;
	}
}

void	fat_lfn_feed(t_flfn *l, const uint8_t *e, uint32_t idx)
{
	uint32_t	n;

	if (e[12] != 0 || le16(e + 26) != 0)
	{
		l->ok = 0;
		return ;
	}
	if (e[0] & FAT_LFN_LAST)
	{
		n = e[0] ^ FAT_LFN_LAST;
		l->ok = (n >= 1 && n <= FAT_LFN_MAX);
		l->count = n;
		l->next = n;
		l->start = idx;
		l->sum = e[13];
	}
	else if (!l->ok || e[0] != l->next || e[13] != l->sum)
		l->ok = 0;
	if (!l->ok)
		return ;
	lfn_store(l, e, l->next);
	l->next--;
}

int	fat_lfn_name(const t_flfn *l, const uint8_t *raw, char *out)
{
	uint32_t	n;
	uint32_t	len;

	if (!l->ok || l->next != 0 || l->sum != fat_lfn_sum(raw))
		return (E_INVAL);
	n = l->count * FAT_LFN_CHARS;
	len = 0;
	while (len < n && l->u[len] != 0)
		len++;
	if (len == 0 || fat_utf16_to8(l->u, len, out, VFS_NAME_MAX + 1) < 0)
		return (E_INVAL);
	if (strcmp(out, ".") == 0 || strcmp(out, "..") == 0)
		return (E_INVAL);
	return (0);
}

void	fat_lfn_build(uint8_t *e, const t_fadd *a, uint32_t ord)
{
	uint32_t	j;
	uint32_t	p;
	uint32_t	v;

	memset(e, 0, FAT_DENT);
	e[0] = (uint8_t)ord;
	if (ord == a->nlfn)
		e[0] |= FAT_LFN_LAST;
	j = 0;
	while (j < FAT_LFN_CHARS)
	{
		p = (ord - 1) * FAT_LFN_CHARS + j;
		v = 0xFFFF;
		if (p < a->ulen)
			v = a->u[p];
		else if (p == a->ulen)
			v = 0;
		put16(e + lfn_off(j), v);
		j++;
	}
	e[11] = FAT_A_LFN;
	e[13] = fat_lfn_sum(a->raw);
}
