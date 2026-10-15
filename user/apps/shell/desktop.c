#include "velum/abi/abi_input.h"
#include "velum/luna.h"
#include "../common/layout.h"
#include "desktop.h"

static const t_deskicon	g_icons[DESK_ICONS] = {
{DA_COMPUTER, ICON_COMPUTER, "Ordinateur"},
{DA_RECYCLE, ICON_RECYCLE, "Corbeille"},
{DA_HELLO, ICON_PROGRAM, "Bonjour"}
};

const t_deskicon	*desk_icon(uint32_t index)
{
	if (index >= DESK_ICONS)
		return (NULL);
	return (&g_icons[index]);
}

int32_t	desk_rows(t_rect area)
{
	int32_t	rows;

	rows = (area.h - DESK_MARGIN - DESK_CELL_H + DESK_STEP_Y) / DESK_STEP_Y;
	if (rows < 1)
		return (0);
	return (rows);
}

int	desk_grid(t_rect area, uint32_t count, t_rect *out)
{
	int32_t		rows;
	uint32_t	i;
	uint32_t	placed;

	rows = desk_rows(area);
	i = 0;
	placed = 0;
	while (rows > 0 && i < count)
	{
		out[i] = lay_rect(area.x + DESK_MARGIN
				+ (int32_t)i / rows * DESK_CELL_W,
				area.y + DESK_MARGIN + (int32_t)i % rows * DESK_STEP_Y,
				DESK_CELL_W, DESK_CELL_H);
		if (lay_inside(area, out[i]))
			placed++;
		i++;
	}
	return ((int)placed);
}

int32_t	desk_hit(const t_rect *cells, uint32_t n, int32_t x, int32_t y)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		if (lay_contains(cells[i], x, y))
			return ((int32_t)i);
		i++;
	}
	return (DESK_NONE);
}

int32_t	desk_move(int32_t sel, uint32_t n, int32_t rows, uint32_t vk)
{
	int32_t	last;

	last = (int32_t)n - 1;
	if (last < 0 || rows < 1)
		return (DESK_NONE);
	if (vk != VK_DOWN && vk != VK_UP && vk != VK_LEFT && vk != VK_RIGHT
		&& vk != VK_HOME && vk != VK_END)
		return (sel);
	if (sel < 0 || sel > last)
		return (0);
	if (vk == VK_DOWN)
		return (lay_clamp(sel + 1, 0, last));
	if (vk == VK_UP)
		return (lay_clamp(sel - 1, 0, last));
	if (vk == VK_RIGHT)
		return (lay_clamp(sel + rows, 0, last));
	if (vk == VK_LEFT)
		return (lay_clamp(sel - rows, 0, last));
	if (vk == VK_HOME)
		return (0);
	if (vk == VK_END)
		return (last);
	return (sel);
}
