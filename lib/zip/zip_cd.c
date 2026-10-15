#include "zip_int.h"

static int	cd_fields(const t_zip *z, uint32_t at, t_zipent *e)
{
	t_span	f;

	f = z->file;
	if (zip_rd32(f, at) != 0x02014b50)
		return (E_INVAL);
	if ((zip_rd16(f, at + 8) & 0x41) != 0 || zip_rd16(f, at + 34) != 0)
		return (E_NOTSUP);
	e->method = zip_rd16(f, at + 10);
	e->crc = zip_rd32(f, at + 16);
	e->csize = zip_rd32(f, at + 20);
	e->usize = zip_rd32(f, at + 24);
	e->name_len = zip_rd16(f, at + 28);
	e->lfh_off = zip_rd32(f, at + 42);
	e->name = (const char *)f.p + at + 46;
	if (e->method != 0 && e->method != 8)
		return (E_NOTSUP);
	if (e->csize == 0xffffffff || e->usize == 0xffffffff
		|| e->lfh_off == 0xffffffff)
		return (E_NOTSUP);
	if (e->name_len > ZIP_NAME_MAX)
		return (E_RANGE);
	if (e->name_len == 0 || (e->method == 0 && e->csize != e->usize))
		return (E_INVAL);
	return (0);
}

static int	cd_same(const uint8_t *a, const char *b, uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		if (a[i] != (uint8_t)b[i])
			return (0);
		i++;
	}
	return (1);
}

static int	cd_local(const t_zip *z, t_zipent *e)
{
	t_span		f;
	uint64_t	end;

	f = z->file;
	e->data_off = 0;
	if ((uint64_t)e->lfh_off + 30 + e->name_len > z->cd_off)
		return (E_INVAL);
	if (zip_rd32(f, e->lfh_off) != 0x04034b50)
		return (E_INVAL);
	if ((zip_rd16(f, e->lfh_off + 6) & 0x41) != 0)
		return (E_NOTSUP);
	if (zip_rd16(f, e->lfh_off + 8) != e->method
		|| zip_rd16(f, e->lfh_off + 26) != e->name_len)
		return (E_INVAL);
	if (!cd_same(f.p + e->lfh_off + 30, e->name, e->name_len))
		return (E_INVAL);
	end = (uint64_t)e->lfh_off + 30 + e->name_len
		+ zip_rd16(f, e->lfh_off + 28);
	if (end + e->csize > z->cd_off)
		return (E_INVAL);
	e->data_off = (uint32_t)end;
	return (0);
}

int	zip_cd_read(const t_zip *z, uint32_t *off, t_zipent *out)
{
	uint64_t	end;
	uint64_t	next;
	int			r;

	end = (uint64_t)z->cd_off + z->cd_size;
	if (*off < z->cd_off || *off > end || end - *off < 46)
		return (E_INVAL);
	next = (uint64_t)(*off) + 46 + zip_rd16(z->file, *off + 28)
		+ zip_rd16(z->file, *off + 30) + zip_rd16(z->file, *off + 32);
	if (next > end)
		return (E_INVAL);
	r = cd_fields(z, *off, out);
	if (r == 0)
		r = cd_local(z, out);
	if (r < 0)
		return (r);
	*off = (uint32_t)next;
	return (0);
}
