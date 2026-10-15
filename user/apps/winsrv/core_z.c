#include "velum/libk.h"
#include "ws_core.h"

uint32_t	wz_band(uint32_t style)
{
	if (style & WS_DESKTOP)
		return (WS_BAND_DESKTOP);
	if (style & (WS_TOPMOST | WS_APPBAR))
		return (WS_BAND_TOP);
	return (WS_BAND_NORMAL);
}

int	wz_index(const t_wtable *t, int slot)
{
	uint32_t	i;

	i = 0;
	while (i < t->nz)
	{
		if (t->z[i] == slot)
			return ((int)i);
		i++;
	}
	return (-1);
}

void	wz_remove(t_wtable *t, int slot)
{
	int	i;

	i = wz_index(t, slot);
	if (i < 0)
		return ;
	memmove(&t->z[i], &t->z[i + 1], (t->nz - (uint32_t)i - 1)
		* sizeof(t->z[0]));
	t->nz--;
}

void	wz_insert(t_wtable *t, int slot)
{
	uint32_t	band;
	uint32_t	i;

	if (t->nz >= WS_WIN_MAX || wz_index(t, slot) >= 0)
		return ;
	band = wz_band(t->w[slot].style);
	i = t->nz;
	while (i > 0 && wz_band(t->w[t->z[i - 1]].style) > band)
		i--;
	memmove(&t->z[i + 1], &t->z[i], (t->nz - i) * sizeof(t->z[0]));
	t->z[i] = (uint16_t)slot;
	t->nz++;
}

bool	wz_raise(t_wtable *t, int slot)
{
	int	before;

	before = wz_index(t, slot);
	if (before < 0)
		return (false);
	wz_remove(t, slot);
	wz_insert(t, slot);
	return (wz_index(t, slot) != before);
}
