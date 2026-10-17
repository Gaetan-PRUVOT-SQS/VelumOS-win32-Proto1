#include <stddef.h>
#include "velum/luna_colors.h"
#include "luna_int.h"

static const size_t	g_off[LUNA_COLORS] = {offsetof(t_lpal, ink),
	offsetof(t_lpal, ink_2), offsetof(t_lpal, muted), offsetof(t_lpal, dim),
	offsetof(t_lpal, accent), offsetof(t_lpal, accent_hi),
	offsetof(t_lpal, accent_text), offsetof(t_lpal, danger),
	offsetof(t_lpal, opaque_content), offsetof(t_lpal, opaque_panel),
	offsetof(t_lpal, sel_bg), offsetof(t_lpal, sel_ring)};

t_color	luna_color(t_lunacolor id)
{
	const uint8_t	*base;

	if ((int)id < 0 || (int)id >= LUNA_COLORS)
		id = LUNA_COLOR_TEXT;
	base = (const uint8_t *)lp_pal();
	return (*(const t_color *)(base + g_off[id]));
}

t_color	luna_color_text_2(void)
{
	return (luna_color(LUNA_COLOR_TEXT_2));
}

t_color	luna_color_muted(void)
{
	return (luna_color(LUNA_COLOR_MUTED));
}

t_color	luna_color_dim(void)
{
	return (luna_color(LUNA_COLOR_DIM));
}

t_color	luna_color_danger(void)
{
	return (luna_color(LUNA_COLOR_DANGER));
}
