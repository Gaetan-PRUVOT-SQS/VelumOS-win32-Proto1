#include <stdio.h>
#include "harness.h"
#include "lt_geo.h"

static void	inv_one(int32_t w, int32_t h, uint32_t style, bool mx)
{
	t_lunawin	win;
	char		msg[96];

	win = lt_win(lp_rect(-7, 11, w, h), style, mx);
	snprintf(msg, sizeof(msg), "invariants %dx%d style 0x%x max %d", w, h,
		style, mx);
	h_true(lt_geo_check(&win), msg);
}

static void	inv_sizes(void)
{
	static const uint32_t	styles[8] = {0x1f, 0x0f, 0x03, 0x01, 0x43, 0x07,
		0x17, 0x00};
	int32_t					w;
	int32_t					h;
	uint32_t				i;

	i = 0;
	while (i < 8)
	{
		w = 0;
		while (w <= 260)
		{
			h = 0;
			while (h <= 90)
			{
				inv_one(w, h, styles[i], false);
				inv_one(w, h, styles[i], true);
				h += 5;
			}
			w++;
		}
		i++;
	}
}

static void	inv_large(void)
{
	inv_one(800, 600, WS_DEFAULT, false);
	inv_one(1024, 768, WS_DEFAULT, true);
	inv_one(LG_DIM_MAX, LG_DIM_MAX, WS_DEFAULT, false);
	inv_one(LG_DIM_MAX + 5, 40, WS_DEFAULT, false);
	inv_one(INT32_MAX, INT32_MAX, WS_DEFAULT, true);
	inv_one(-1, -1, WS_DEFAULT, false);
}

int	main(void)
{
	h_begin("a17/geo_inv");
	h_run("invariants sur 0..260 x 0..90", inv_sizes);
	h_run("grandes et absurdes", inv_large);
	return (h_end());
}
