#include "sys_int.h"
#include "velum/vdisplay.h"

int	v_display_info(t_dispinfo *out)
{
	return ((int)sys1(SYS_DISPLAY_INFO, sys_ptr(out)));
}

int64_t	v_display_map(uint64_t hint_va)
{
	return (sys1(SYS_DISPLAY_MAP, hint_va));
}

int	v_display_set_mode(uint32_t width, uint32_t height, uint32_t bpp)
{
	return ((int)sys3(SYS_DISPLAY_SET_MODE, width, height, bpp));
}

int	v_display_modes(t_dispmode *out, uint32_t max)
{
	return ((int)sys2(SYS_DISPLAY_MODES, sys_ptr(out), max));
}

int	v_kcon(int enable)
{
	return ((int)sys1(SYS_KCON, enable != 0));
}
