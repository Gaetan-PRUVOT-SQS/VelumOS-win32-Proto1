#include "zip_int.h"

static int	eocd_find(t_span f, size_t *pos)
{
	size_t	i;
	size_t	low;

	*pos = 0;
	if (f.p == NULL || f.len < 22 || f.len > 0xffffffff)
		return (E_INVAL);
	low = 0;
	if (f.len > 22 + 65535)
		low = f.len - 22 - 65535;
	i = f.len - 22;
	while (1)
	{
		if (zip_rd32(f, i) == 0x06054b50
			&& zip_rd16(f, i + 20) == f.len - i - 22)
		{
			*pos = i;
			return (0);
		}
		if (i == low)
			return (E_INVAL);
		i--;
	}
}

static int	eocd_parse(t_zip *z, size_t pos)
{
	t_span	f;

	f = z->file;
	if (pos >= 20 && zip_rd32(f, pos - 20) == 0x07064b50)
		return (E_NOTSUP);
	if (zip_rd16(f, pos + 4) != 0 || zip_rd16(f, pos + 6) != 0
		|| zip_rd16(f, pos + 8) != zip_rd16(f, pos + 10))
		return (E_NOTSUP);
	z->count = zip_rd16(f, pos + 10);
	z->cd_size = zip_rd32(f, pos + 12);
	z->cd_off = zip_rd32(f, pos + 16);
	if (z->count == 0xffff || z->cd_size == 0xffffffff
		|| z->cd_off == 0xffffffff)
		return (E_NOTSUP);
	if ((uint64_t)z->cd_off + z->cd_size != pos)
		return (E_INVAL);
	if (z->count > ZIP_MAX_ENTRIES)
		return (E_RANGE);
	return (0);
}

static int	zip_dup(const t_zip *z, const t_zipent *e, uint32_t index)
{
	t_zipent	o;
	uint32_t	off;
	uint32_t	j;
	uint32_t	k;

	off = z->cd_off;
	j = 0;
	while (j < index)
	{
		if (zip_cd_read(z, &off, &o) < 0)
			return (E_INVAL);
		k = 0;
		while (o.name_len == e->name_len && k < o.name_len
			&& o.name[k] == e->name[k])
			k++;
		if (o.name_len == e->name_len && k == o.name_len)
			return (E_INVAL);
		j++;
	}
	return (0);
}

int	zip_open(t_zip *z, t_span file)
{
	t_zipent	e;
	size_t		pos;
	uint32_t	off;
	uint32_t	i;
	int			r;

	z->file = file;
	z->cd_off = 0;
	r = eocd_find(file, &pos);
	if (r == 0)
		r = eocd_parse(z, pos);
	off = z->cd_off;
	i = 0;
	while (r == 0 && i < z->count)
	{
		r = zip_cd_read(z, &off, &e);
		if (r == 0)
			r = zip_dup(z, &e, i);
		i++;
	}
	if (r == 0 && off != pos)
		r = E_INVAL;
	if (r < 0)
		z->count = 0;
	return (r);
}
