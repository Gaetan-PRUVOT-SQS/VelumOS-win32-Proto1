#include "cpio.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	hex_digit(uint8_t c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (-1);
}

static int	hdr_fields(const uint8_t *h, uint32_t *f)
{
	int	i;
	int	j;
	int	d;

	i = 0;
	while (i < CPIO_FIELDS)
	{
		f[i] = 0;
		j = 0;
		while (j < 8)
		{
			d = hex_digit(h[6 + i * 8 + j]);
			if (d < 0)
				return (E_INVAL);
			f[i] = (f[i] << 4) | (uint32_t)d;
			j++;
		}
		i++;
	}
	return (0);
}

static int	hdr_name(const uint8_t *b, uint64_t size, uint64_t o, uint32_t ns)
{
	const char	*name;

	if (ns == 0 || ns > VFS_PATH_MAX || size - o - CPIO_HDR < ns)
		return (E_INVAL);
	name = (const char *)b + o + CPIO_HDR;
	if (name[ns - 1] != '\0' || strnlen(name, ns) != ns - 1)
		return (E_INVAL);
	return (0);
}

static void	ent_fill(t_cpent *e, const uint8_t *b, uint64_t o,
		const uint32_t *f)
{
	e->name = (const char *)b + o + CPIO_HDR;
	e->nlen = f[11] - 1;
	e->mode = f[1];
	e->mtime = f[5];
	e->size = f[6];
	e->order = 0;
	e->pad = 0;
}

int	cpio_next(const uint8_t *b, uint64_t size, uint64_t *off, t_cpent *e)
{
	uint32_t	f[CPIO_FIELDS];
	uint64_t	o;
	uint64_t	data;

	o = *off;
	if (o > size || size - o < CPIO_HDR)
		return (E_INVAL);
	if (memcmp(b + o, "070701", 6) != 0 || hdr_fields(b + o, f) < 0)
		return (E_INVAL);
	if (hdr_name(b, size, o, f[11]) < 0)
		return (E_INVAL);
	data = (o + CPIO_HDR + f[11] + 3) & ~3ull;
	if (data > size || size - data < f[6])
		return (E_INVAL);
	ent_fill(e, b, o, f);
	e->data = b + data;
	*off = (data + f[6] + 3) & ~3ull;
	if (*off > size)
		*off = size;
	return (strcmp(e->name, "TRAILER!!!") != 0);
}
