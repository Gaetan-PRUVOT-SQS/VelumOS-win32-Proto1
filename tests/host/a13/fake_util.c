#include "fakes.h"
#include "velum/boot.h"
#include "velum/libk.h"

void	fake_fb_set(uint32_t w, uint32_t h)
{
	t_fbinfo	*fb;

	fb = &boot_info_rw()->fb;
	memset(fb, 0, sizeof(*fb));
	fb->phys = FAKE_FB_PHYS;
	fb->virt = FAKE_FB_PHYS + boot_info()->hhdm;
	fb->width = w;
	fb->height = h;
	fb->pitch = w * 4;
	fb->bpp = 32;
	fb->red_shift = 16;
	fb->red_size = 8;
	fb->green_shift = 8;
	fb->green_size = 8;
	fb->blue_size = 8;
}

void	fake_cmdline(const char *s)
{
	strlcpy(boot_info_rw()->cmdline, s, BOOT_CMDLINE_MAX);
}

uint32_t	fake_count(const t_surface *s, t_rect r, t_color c)
{
	uint32_t	n;
	t_point		p;

	n = 0;
	p.y = r.y;
	while (p.y < r.y + r.h)
	{
		p.x = r.x;
		while (p.x < r.x + r.w)
		{
			if (gfx_get(s, p) == c)
				n++;
			p.x++;
		}
		p.y++;
	}
	return (n);
}
