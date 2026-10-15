#include "lt_hit.h"

bool	lt_ht_valid(uint32_t h)
{
	return (h == HT_NOWHERE || h == HT_CLIENT || h == HT_CAPTION
		|| h == HT_SYSMENU || h == HT_MINBUTTON || h == HT_MAXBUTTON
		|| h == HT_CLOSE || (h >= HT_LEFT && h <= HT_BOTTOMRIGHT));
}

bool	lt_ht_edge(uint32_t h)
{
	return (h >= HT_LEFT && h <= HT_BOTTOMRIGHT);
}

uint32_t	lt_ht_mirror(uint32_t h)
{
	if (h == HT_LEFT)
		return (HT_RIGHT);
	if (h == HT_RIGHT)
		return (HT_LEFT);
	if (h == HT_TOPLEFT)
		return (HT_TOPRIGHT);
	if (h == HT_TOPRIGHT)
		return (HT_TOPLEFT);
	if (h == HT_BOTTOMLEFT)
		return (HT_BOTTOMRIGHT);
	if (h == HT_BOTTOMRIGHT)
		return (HT_BOTTOMLEFT);
	return (h);
}

uint32_t	lt_area(t_rect r)
{
	if (r.w <= 0 || r.h <= 0)
		return (0);
	return ((uint32_t)r.w * (uint32_t)r.h);
}

uint32_t	lt_rep_style(uint32_t i)
{
	static const uint32_t	frameless[5] = {0, 0x200, 0x400, 0x800, 0xe00};
	uint32_t				combo;

	combo = i % 64;
	combo = (combo & 0x0f) | ((combo & 0x10) << 0) | ((combo & 0x20) << 1);
	return (combo | frameless[(i / 64) % 5]);
}
