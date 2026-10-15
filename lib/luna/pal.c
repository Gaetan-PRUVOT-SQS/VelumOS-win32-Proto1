#include "velum/theme.h"
#include "luna_int.h"

static const t_lpal	g_light = {VT_ACCENT, VT_ACCENT_HI, VT_ACCENT,
	VT_ACCENT_SEL_BG, VT_ACCENT_SEL_RING, VT_INK, VT_INK_2, VT_MUTED, VT_DIM,
	VT_DANGER, VT_GLASS_FRAME, VT_GLASS_DOCK, VT_GLASS_CHIP, VT_GLASS_PANEL,
	VT_GLASS_WINDOW, VT_GLASS_CONTENT, VT_GLASS_CONTROL, VT_GLASS_ITEM_SEL,
	VT_GLASS_INPUT, 0x66ffffff, VT_GLASS_ITEM_SEL, VT_RIM_TOP, VT_RIM_IN,
	VT_RIM_BOTTOM, VT_SPEC_TOP, VT_SHADOW_WIN, VT_SHADOW_DOCK,
	VT_OPAQUE_PANEL, VT_OPAQUE_CONTENT, {VT_WALL_BASE, VT_WALL_TEAL,
	VT_WALL_VIOLET, VT_WALL_AMBER, VT_WALL_ROSE}, VT_SAT_LIGHT_X10,
	VT_SAT_PANEL_LIGHT_X10, 191, 140};
static const t_lpal	g_dark = {VT_ACCENT_HI, VT_ACCENT_HI,
	VT_ACCENT_TEXT_DARK, VT_ACCENT_SEL_BG_D, VT_ACCENT_SEL_RING_D, VT_INK_D,
	VT_INK_2_D, VT_MUTED_D, VT_DIM_D, VT_DANGER_D, VT_GLASS_FRAME_D,
	VT_GLASS_DOCK_D, VT_GLASS_CHIP_D, VT_GLASS_PANEL_D, VT_GLASS_WINDOW_D,
	VT_GLASS_CONTENT_D, VT_GLASS_CONTROL_D, VT_GLASS_ITEM_SEL_D,
	VT_GLASS_INPUT_D, 0x14ffffff, 0x24ffffff, VT_RIM_TOP_D, VT_RIM_IN_D,
	VT_RIM_BOTTOM_D, VT_SPEC_TOP_D, VT_SHADOW_WIN_D, VT_SHADOW_DOCK_D,
	VT_OPAQUE_PANEL_D, VT_OPAQUE_CONTENT_D, {VT_WALL_BASE_D,
	VT_WALL_TEAL_D, VT_WALL_VIOLET_D, VT_WALL_AMBER_D, VT_WALL_ROSE_D},
	VT_SAT_DARK_X10, VT_SAT_PANEL_DARK_X10, 36, 26};
static t_lunatheme	g_theme = LUNA_THEME_LIGHT;
static bool			g_reduced;

t_cpal	lp_pal(void)
{
	if (g_theme == LUNA_THEME_DARK)
		return (&g_dark);
	return (&g_light);
}

void	luna_theme_set(t_lunatheme theme)
{
	if (theme == LUNA_THEME_DARK)
		g_theme = LUNA_THEME_DARK;
	else
		g_theme = LUNA_THEME_LIGHT;
}

t_lunatheme	luna_theme_get(void)
{
	return (g_theme);
}

void	luna_transparency_set(bool reduced)
{
	g_reduced = reduced;
}

bool	luna_transparency_reduced(void)
{
	return (g_reduced);
}
