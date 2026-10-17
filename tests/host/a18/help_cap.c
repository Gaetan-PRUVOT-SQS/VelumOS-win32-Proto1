#include "help.h"
#include "velum/theme.h"

int32_t	hs_capbtn_x(int32_t win_x, int32_t win_w, int rank)
{
	int32_t	x;

	x = win_x + win_w - 10 - VT_CAPGROUP_PAD - VT_CAPBTN_W / 2;
	return (x - rank * (VT_CAPBTN_W + VT_CAPGROUP_GAP));
}

int32_t	hs_capbtn_y(int32_t win_y)
{
	return (win_y + VT_CAPTION_H / 2);
}
