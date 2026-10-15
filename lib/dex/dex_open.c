#include "dex_int.h"

static const uint8_t	g_item[6] = {4, 4, 12, 8, 8, 32};

static int	check_header(const t_dex *d)
{
	const uint8_t	*p;

	p = d->p;
	if (d->len < 0x70 || d->len > 0xffffffffu)
		return (E_INVAL);
	if (p[0] != 'd' || p[1] != 'e' || p[2] != 'x' || p[3] != '\n')
		return (E_INVAL);
	if (p[4] != '0' || p[5] != '3' || p[6] < '5' || p[6] > '9' || p[7] != 0)
		return (E_INVAL);
	if (dex_u32(p + 32) != d->len || dex_u32(p + 36) != 0x70)
		return (E_INVAL);
	if (dex_u32(p + 40) == 0x78563412)
		return (E_NOTSUP);
	if (dex_u32(p + 40) != 0x12345678)
		return (E_INVAL);
	if (dex_u32(p + 8) != dex_adler32(p + 12, d->len - 12))
		return (E_INVAL);
	return (E_OK);
}

static int	load_tables(t_dex *d)
{
	uint32_t	i;

	i = 0;
	while (i < 6)
	{
		d->n[i] = dex_u32(d->p + 56 + 8 * i);
		d->off[i] = dex_u32(d->p + 60 + 8 * i);
		if (d->n[i] != 0 && (d->off[i] < 0x70 || (d->off[i] & 3)
				|| !dex_fits(d, d->off[i], d->n[i], g_item[i])))
			return (E_INVAL);
		i++;
	}
	if (d->n[DEX_T_TYPE] > 0xffff || d->n[DEX_T_PROTO] > 0xffff)
		return (E_INVAL);
	return (E_OK);
}

static int	check_areas(const t_dex *d)
{
	uint32_t	map;

	if (!dex_fits(d, dex_u32(d->p + 108), dex_u32(d->p + 104), 1))
		return (E_INVAL);
	if (dex_u32(d->p + 44) != 0
		&& !dex_fits(d, dex_u32(d->p + 48), dex_u32(d->p + 44), 1))
		return (E_INVAL);
	map = dex_u32(d->p + 52);
	if (map < 0x70 || (map & 3) || !dex_fits(d, map, 4, 1))
		return (E_INVAL);
	if (!dex_fits(d, (uint64_t)map + 4, dex_u32(d->p + map), 12))
		return (E_INVAL);
	return (E_OK);
}

static void	wipe(t_dex *d)
{
	uint32_t	i;

	d->p = 0;
	d->len = 0;
	i = 0;
	while (i < 6)
	{
		d->n[i] = 0;
		d->off[i] = 0;
		i++;
	}
}

int	dex_open(t_dex *d, t_span file)
{
	int	r;

	if (!d)
		return (E_INVAL);
	wipe(d);
	if (!file.p)
		return (E_INVAL);
	d->p = file.p;
	d->len = file.len;
	r = check_header(d);
	if (r == E_OK)
		r = load_tables(d);
	if (r == E_OK)
		r = check_areas(d);
	if (r == E_OK)
		r = dex_verify_ids(d);
	if (r == E_OK)
		r = dex_verify_classes(d);
	if (r != E_OK)
		wipe(d);
	return (r);
}
