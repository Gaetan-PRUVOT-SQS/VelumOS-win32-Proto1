#include "fake.h"
#include "dex_int.h"

int	fake_same(const char *a, const char *b)
{
	size_t	i;

	i = 0;
	while (a[i] != 0 && a[i] == b[i])
		i++;
	return (a[i] == b[i]);
}

int	fake_named(const t_dex *d, uint32_t string_idx, const char *want)
{
	t_dexstr	s;

	if (dex_string(d, string_idx, &s) != E_OK)
		return (0);
	return (fake_same(s.p, want));
}

int	fake_string_idx(const t_dex *d, const char *want)
{
	uint32_t	i;

	i = 0;
	while (i < d->n[DEX_T_STRING])
	{
		if (fake_named(d, i, want))
			return ((int)i);
		i++;
	}
	return (-1);
}
