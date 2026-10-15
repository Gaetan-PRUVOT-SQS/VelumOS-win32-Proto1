#include "velum/luna.h"
#include "layout.h"
#include "logon_ink.h"

static uint32_t	g_ref[SCREEN_W * SCREEN_H];

static int32_t	delta(uint32_t a, uint32_t b)
{
	int32_t	d;

	d = (int32_t)((a >> 16) & 0xffu) - (int32_t)((b >> 16) & 0xffu);
	if (d < 0)
		d = -d;
	return (d);
}

void	lgi_ref(void)
{
	t_surface	ref;

	ref = (t_surface){g_ref, SCREEN_W, SCREEN_H, SCREEN_W,
		rect_make(0, 0, SCREEN_W, SCREEN_H)};
	luna_logon_bg(&ref, ref.clip);
}

static void	grow(int32_t k[4], int32_t x, int32_t y)
{
	if (y < k[LGI_TOP])
		k[LGI_TOP] = y;
	if (y > k[LGI_BOTTOM])
		k[LGI_BOTTOM] = y;
	if (x < k[LGI_LEFT])
		k[LGI_LEFT] = x;
	if (x > k[LGI_RIGHT])
		k[LGI_RIGHT] = x;
}

void	lgi_scan(const t_logon *lg, t_rect band, int32_t k[4])
{
	const t_surface	*s;
	int32_t			x;
	int32_t			y;

	s = &lg->ui.win.surface;
	k[LGI_TOP] = band.y + band.h;
	k[LGI_BOTTOM] = -1;
	k[LGI_LEFT] = band.x + band.w;
	k[LGI_RIGHT] = -1;
	y = band.y;
	while (y < band.y + band.h)
	{
		x = band.x;
		while (x < band.x + band.w)
		{
			if (delta(s->px[y * s->stride + x], g_ref[y * SCREEN_W + x]) > 90)
				grow(k, x, y);
			x++;
		}
		y++;
	}
}
