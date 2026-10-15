#include "dex_int.h"

static const uint8_t	g_item[6] = {4, 4, 12, 8, 8, 32};

const uint8_t	*dex_item(const t_dex *d, uint32_t table, uint32_t i)
{
	return (d->p + d->off[table] + (size_t)i * g_item[table]);
}

int	dex_type_list(const t_dex *d, uint32_t off, t_dexlist *out)
{
	uint32_t	i;

	if (!d || !out)
		return (E_INVAL);
	out->p = 0;
	out->n = 0;
	if (off == 0)
		return (E_OK);
	if ((off & 3) || !dex_fits(d, off, 4, 1))
		return (E_INVAL);
	if (!dex_fits(d, (uint64_t)off + 4, dex_u32(d->p + off), 2))
		return (E_INVAL);
	i = 0;
	while (i < dex_u32(d->p + off))
	{
		if (dex_u16(d->p + off + 4 + 2 * (size_t)i) >= d->n[DEX_T_TYPE])
			return (E_INVAL);
		i++;
	}
	out->p = d->p + off + 4;
	out->n = i;
	return (E_OK);
}

uint32_t	dex_list_at(const t_dexlist *l, uint32_t i)
{
	if (!l || i >= l->n)
		return (DEX_NO_INDEX);
	return (dex_u16(l->p + 2 * (size_t)i));
}
