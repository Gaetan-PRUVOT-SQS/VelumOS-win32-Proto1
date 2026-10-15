#include "cases.h"
#include "fakes.h"

static void	set_color(t_fbinfo *fb, t_geofield f, uint8_t v)
{
	if (f == F_RS)
		fb->red_shift = v;
	else if (f == F_RZ)
		fb->red_size = v;
	else if (f == F_GS)
		fb->green_shift = v;
	else if (f == F_GZ)
		fb->green_size = v;
	else if (f == F_BS)
		fb->blue_shift = v;
	else if (f == F_BZ)
		fb->blue_size = v;
}

void	fake_fb_field(t_fbinfo *fb, t_geofield f, uint64_t v)
{
	if (f == F_W)
		fb->width = (uint32_t)v;
	else if (f == F_H)
		fb->height = (uint32_t)v;
	else if (f == F_PITCH)
		fb->pitch = (uint32_t)v;
	else if (f == F_BPP)
		fb->bpp = (uint32_t)v;
	else if (f == F_PHYS)
		fb->phys = v;
	else
		set_color(fb, f, (uint8_t)v);
}
