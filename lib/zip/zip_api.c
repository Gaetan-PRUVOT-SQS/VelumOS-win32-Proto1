#include "zip_int.h"

int	zip_entry(const t_zip *z, uint32_t index, t_zipent *out)
{
	uint32_t	off;
	uint32_t	i;
	int			r;

	if (index >= z->count)
		return (E_RANGE);
	off = z->cd_off;
	i = 0;
	r = 0;
	while (r == 0 && i <= index)
	{
		r = zip_cd_read(z, &off, out);
		i++;
	}
	return (r);
}

int	zip_find(const t_zip *z, const char *name, t_zipent *out)
{
	uint32_t	off;
	uint32_t	i;
	uint32_t	k;

	off = z->cd_off;
	i = 0;
	while (i < z->count)
	{
		if (zip_cd_read(z, &off, out) < 0)
			return (E_INVAL);
		k = 0;
		while (k < out->name_len && name[k] != '\0'
			&& name[k] == out->name[k])
			k++;
		if (k == out->name_len && name[k] == '\0')
			return (0);
		i++;
	}
	return (E_NOENT);
}

int	zip_name_safe(const char *name, size_t len)
{
	size_t	i;
	size_t	start;

	if (name == NULL || len == 0 || name[0] == '/')
		return (0);
	i = 0;
	start = 0;
	while (i <= len)
	{
		if (i < len && (name[i] == '\0' || name[i] == '\\'))
			return (0);
		if (i == len || name[i] == '/')
		{
			if (i - start == 2 && name[start] == '.'
				&& name[start + 1] == '.')
				return (0);
			start = i + 1;
		}
		i++;
	}
	return (1);
}

static int64_t	zip_stored(const t_zip *z, const t_zipent *e, uint8_t *out)
{
	uint32_t	i;

	if (e->csize != e->usize)
		return (E_INVAL);
	i = 0;
	while (i < e->usize)
	{
		out[i] = z->file.p[e->data_off + i];
		i++;
	}
	return (e->usize);
}

int64_t	zip_extract(const t_zip *z, const t_zipent *e, uint8_t *out,
		size_t cap)
{
	t_span	in;
	int64_t	n;

	if ((uint64_t)e->data_off + e->csize > z->file.len)
		return (E_INVAL);
	if (e->usize > cap)
		return (E_OVERFLOW);
	in.p = z->file.p + e->data_off;
	in.len = e->csize;
	if (e->method == 0)
		n = zip_stored(z, e, out);
	else if (e->method == 8)
		n = inflate_raw(in, out, e->usize);
	else
		return (E_NOTSUP);
	if (n == E_OVERFLOW || (n >= 0 && n != (int64_t)e->usize))
		return (E_INVAL);
	if (n >= 0 && zip_crc32(0, out, e->usize) != e->crc)
		return (E_INVAL);
	return (n);
}
