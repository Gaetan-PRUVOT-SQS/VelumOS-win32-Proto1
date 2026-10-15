#include "luna_ctrl.h"

bool	lp_dark(void)
{
	return (luna_theme_get() == LUNA_THEME_DARK);
}

t_color	lp_tone_alpha(t_color c, uint32_t a)
{
	return ((c & 0x00ffffffU) | (a << 24));
}

t_color	lp_tone_hover(void)
{
	if (lp_dark())
		return (0x14ffffff);
	return (0x66ffffff);
}

t_color	lp_tone_press(void)
{
	if (lp_dark())
		return (0x24ffffff);
	return (3019898879U);
}

t_color	lp_tone_track(void)
{
	if (lp_dark())
		return (0x1affffff);
	return (0x1f141824);
}
