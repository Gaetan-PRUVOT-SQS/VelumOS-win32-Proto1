#include "cpio.h"
#include "velum/err.h"

static int	comps_ok(const char *p, uint32_t n)
{
	uint32_t	i;
	uint32_t	len;

	i = 0;
	while (i <= n)
	{
		len = 0;
		while (i + len < n && p[i + len] != '/')
			len++;
		if (len == 0 || len > VFS_NAME_MAX)
			return (0);
		if (len == 1 && p[i] == '.')
			return (0);
		if (len == 2 && p[i] == '.' && p[i + 1] == '.')
			return (0);
		i += len + 1;
	}
	return (1);
}

static int	strip_lead(const char **p, uint32_t *n)
{
	if (*n >= 2 && (*p)[0] == '.' && (*p)[1] == '/')
	{
		*p += 2;
		*n -= 2;
		return (1);
	}
	if (*n && (*p)[0] == '/')
	{
		(*p)++;
		(*n)--;
		return (1);
	}
	return (0);
}

int	cpio_name_norm(t_cpent *e)
{
	const char	*p;
	uint32_t	n;

	p = e->name;
	n = e->nlen;
	while (strip_lead(&p, &n))
		;
	while (n && p[n - 1] == '/')
		n--;
	if (n == 0 || (n == 1 && p[0] == '.'))
		return (0);
	if (n >= VFS_PATH_MAX - 1 || !comps_ok(p, n))
		return (E_INVAL);
	if (vpath_utf8_ok((const uint8_t *)p, n) < 0)
		return (E_INVAL);
	if ((e->mode & CPIO_TYPE_MASK) != CPIO_TYPE_DIR
		&& (e->mode & CPIO_TYPE_MASK) != CPIO_TYPE_REG)
		return (0);
	e->name = p;
	e->nlen = n;
	return (1);
}

uint32_t	cpio_lower(const t_initrd *rd, const char *key, uint32_t klen)
{
	uint32_t	lo;
	uint32_t	hi;
	uint32_t	mid;

	lo = 0;
	hi = rd->n;
	while (lo < hi)
	{
		mid = lo + (hi - lo) / 2;
		if (cpio_cmp(rd->ents[mid].name, rd->ents[mid].nlen, key, klen) < 0)
			lo = mid + 1;
		else
			hi = mid;
	}
	return (lo);
}

uint64_t	cpio_hash(uint64_t h, const char *s, uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		h ^= (uint8_t)s[i];
		h *= FNV_PRIME;
		i++;
	}
	return (h);
}
