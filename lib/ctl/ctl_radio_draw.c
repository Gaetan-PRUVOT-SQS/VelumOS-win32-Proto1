#include "ctl_int.h"
#include "velum/theme.h"

#define RAD_SUB 4
#define RAD_EIGHTH 8
#define RAD_RING_THIN 12
#define RAD_RING_THICK 32
#define RAD_BORDER_LIGHT 0xff8a93a8u
#define RAD_BORDER_DARK 0xff5d667au

typedef struct s_radcover
{
	uint32_t	ring;
	uint32_t	inside;
}	t_radcover;

static t_color	rad_ring_color(t_lunastate st, bool on, bool dark)
{
	t_color	c;

	c = dark ? RAD_BORDER_DARK : RAD_BORDER_LIGHT;
	if (on || st == LS_HOT)
		c = dark ? VT_ACCENT_HI : VT_ACCENT;
	if (st == LS_DISABLED)
		c = (c & 0x00ffffffu) | 0x7f000000u;
	return (c);
}

static t_radcover	rad_cover(int32_t px, int32_t py, int32_t ring8)
{
	t_radcover	cv;
	int32_t		i;
	int32_t		j;
	int32_t		d2;
	int32_t		in;

	cv.ring = 0;
	cv.inside = 0;
	in = (CTL_RADIO_BOX / 2) * RAD_EIGHTH - ring8;
	i = 0;
	while (i < RAD_SUB * RAD_SUB)
	{
		j = (px * RAD_SUB + i % RAD_SUB) * 2 + 1 - CTL_RADIO_BOX * 4;
		d2 = (py * RAD_SUB + i / RAD_SUB) * 2 + 1 - CTL_RADIO_BOX * 4;
		d2 = j * j + d2 * d2;
		if (d2 <= in * in)
			cv.inside++;
		else if (d2 <= (CTL_RADIO_BOX * 4) * (CTL_RADIO_BOX * 4))
			cv.ring++;
		i++;
	}
	return (cv);
}

static t_color	rad_alpha(t_color c, uint32_t cover)
{
	uint32_t	a;

	a = (c >> 24) * cover / (RAD_SUB * RAD_SUB);
	return ((c & 0x00ffffffu) | (a << 24));
}

void	ctl_radio_draw(t_surface *s, t_point p, t_lunastate st, bool on)
{
	t_radcover	cv;
	bool		dark;
	t_color		fill;
	t_point		q;

	dark = luna_theme_get() == LUNA_THEME_DARK;
	fill = dark ? VT_OPAQUE_CONTENT_D : VT_OPAQUE_CONTENT;
	q.y = 0;
	while (q.y < CTL_RADIO_BOX)
	{
		q.x = 0;
		while (q.x < CTL_RADIO_BOX)
		{
			cv = rad_cover(q.x, q.y, on ? RAD_RING_THICK : RAD_RING_THIN);
			if (luna_transparency_reduced() && cv.inside)
				gfx_put(s, (t_point){p.x + q.x, p.y + q.y},
					rad_alpha(fill, cv.inside));
			if (cv.ring)
				gfx_put(s, (t_point){p.x + q.x, p.y + q.y},
					rad_alpha(rad_ring_color(st, on, dark), cv.ring));
			q.x++;
		}
		q.y++;
	}
}
