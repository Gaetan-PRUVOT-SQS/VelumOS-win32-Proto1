#include "sh_desk.h"

static int32_t	isqrt32(uint32_t v)
{
	uint32_t	r;

	r = 0;
	while ((r + 1) * (r + 1) <= v)
		r++;
	return ((int32_t)r);
}

int32_t	sh_sel_depth(t_rect cell, int32_t x, int32_t y)
{
	int32_t	dx;
	int32_t	dy;
	int32_t	r;

	dx = x * 16 + 8;
	dy = y * 16 + 8;
	if (cell.w * 16 - dx < dx)
		dx = cell.w * 16 - dx;
	if (cell.h * 16 - dy < dy)
		dy = cell.h * 16 - dy;
	r = SH_SEL_RADIUS * 16;
	if (dx >= r || dy >= r)
	{
		if (dy < dx)
			return (dy);
		return (dx);
	}
	dx = r - dx;
	dy = r - dy;
	return (r - isqrt32((uint32_t)(dx * dx + dy * dy)));
}

static int32_t	clamp16(int32_t v)
{
	if (v < 0)
		return (0);
	if (v > 16)
		return (16);
	return (v);
}

static void	sel_pixel(const t_selpaint *k, int32_t x, int32_t y)
{
	int32_t		depth;
	uint32_t	cover;
	uint32_t	ring;
	t_point		at;

	depth = sh_sel_depth(k->cell, x, y);
	cover = (uint32_t)clamp16(depth + 8);
	ring = (uint32_t)clamp16(32 - depth);
	at = (t_point){k->cell.x + x, k->cell.y + y};
	gfx_put(k->s, at, ((SH_SEL_FILL_A * cover / 16) << 24) | k->rgb);
	gfx_put(k->s, at, ((k->ring_a * cover * ring / 256) << 24) | k->rgb);
}

void	sh_sel_paint(const t_selpaint *k)
{
	int32_t	x;
	int32_t	y;

	if (!k || !k->s)
		return ;
	y = 0;
	while (y < k->cell.h)
	{
		x = 0;
		while (x < k->cell.w)
		{
			sel_pixel(k, x, y);
			x++;
		}
		y++;
	}
}
