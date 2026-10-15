#include <string.h>
#include "ref.h"

int	rg_paint(uint8_t cells[RGN_G * RGN_G], const t_region *rg)
{
	uint32_t	i;
	int32_t		x;
	int32_t		y;
	int			twice;

	memset(cells, 0, RGN_G * RGN_G);
	twice = 0;
	i = 0;
	while (i < rg->n)
	{
		y = rg->r[i].y;
		while (y < rg->r[i].y + rg->r[i].h)
		{
			x = rg->r[i].x;
			while (x < rg->r[i].x + rg->r[i].w)
			{
				twice += cells[y * RGN_G + x] != 0;
				cells[y * RGN_G + x] = 1;
				x++;
			}
			y++;
		}
		i++;
	}
	return (twice);
}

void	rg_model(uint8_t cells[RGN_G * RGN_G], int op, t_rect r)
{
	int32_t	x;
	int32_t	y;
	bool	in;

	y = 0;
	while (y < RGN_G)
	{
		x = 0;
		while (x < RGN_G)
		{
			in = in_rect(r, x, y);
			if (op == 0 && in)
				cells[y * RGN_G + x] = 1;
			if (op == 1 && in)
				cells[y * RGN_G + x] = 0;
			if (op == 2 && !in)
				cells[y * RGN_G + x] = 0;
			x++;
		}
		y++;
	}
}

void	rg_apply(t_region *rg, int op, t_rect r)
{
	if (op == 0)
		region_add(rg, r);
	else if (op == 1)
		region_subtract(rg, r);
	else
		region_intersect(rg, r);
}

bool	rg_joinable(t_rect a, t_rect b)
{
	if (a.y == b.y && a.h == b.h && (a.x + a.w == b.x || b.x + b.w == a.x))
		return (true);
	if (a.x == b.x && a.w == b.w && (a.y + a.h == b.y || b.y + b.h == a.y))
		return (true);
	return (false);
}

bool	rg_valid(const t_region *rg)
{
	uint32_t	i;
	uint32_t	j;

	if (rg->n > GFX_REGION_MAX)
		return (false);
	i = 0;
	while (i < rg->n)
	{
		if (rect_empty(rg->r[i]))
			return (false);
		j = i + 1;
		while (j < rg->n)
		{
			if (rg_joinable(rg->r[i], rg->r[j]))
				return (false);
			j++;
		}
		i++;
	}
	return (true);
}
