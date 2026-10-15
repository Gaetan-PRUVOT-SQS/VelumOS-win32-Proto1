#include "dex_int.h"

static int	verify_strings(const t_dex *d)
{
	uint32_t	i;
	t_dexstr	s;

	i = 0;
	while (i < d->n[DEX_T_STRING])
	{
		if (dex_string(d, i, &s) != E_OK)
			return (E_INVAL);
		i++;
	}
	return (E_OK);
}

static int	verify_types(const t_dex *d)
{
	uint32_t	i;

	i = 0;
	while (i < d->n[DEX_T_TYPE])
	{
		if (dex_u32(dex_item(d, DEX_T_TYPE, i)) >= d->n[DEX_T_STRING])
			return (E_INVAL);
		i++;
	}
	return (E_OK);
}

static int	verify_protos(const t_dex *d)
{
	uint32_t		i;
	const uint8_t	*p;
	t_dexlist		l;

	i = 0;
	while (i < d->n[DEX_T_PROTO])
	{
		p = dex_item(d, DEX_T_PROTO, i);
		if (dex_u32(p) >= d->n[DEX_T_STRING]
			|| dex_u32(p + 4) >= d->n[DEX_T_TYPE]
			|| dex_type_list(d, dex_u32(p + 8), &l) != E_OK)
			return (E_INVAL);
		i++;
	}
	return (E_OK);
}

static int	verify_members(const t_dex *d, uint32_t table, uint32_t mid_n)
{
	uint32_t		i;
	const uint8_t	*p;

	i = 0;
	while (i < d->n[table])
	{
		p = dex_item(d, table, i);
		if (dex_u16(p) >= d->n[DEX_T_TYPE] || dex_u16(p + 2) >= mid_n
			|| dex_u32(p + 4) >= d->n[DEX_T_STRING])
			return (E_INVAL);
		i++;
	}
	return (E_OK);
}

int	dex_verify_ids(const t_dex *d)
{
	if (verify_strings(d) != E_OK || verify_types(d) != E_OK)
		return (E_INVAL);
	if (verify_protos(d) != E_OK)
		return (E_INVAL);
	if (verify_members(d, DEX_T_FIELD, d->n[DEX_T_TYPE]) != E_OK)
		return (E_INVAL);
	return (verify_members(d, DEX_T_METHOD, d->n[DEX_T_PROTO]));
}
