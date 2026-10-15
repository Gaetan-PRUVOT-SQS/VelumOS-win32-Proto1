#include "display_int.h"
#include "velum/cpu.h"
#include "velum/err.h"
#include "velum/util.h"
#include "velum/vmm.h"

static void	*map_fb(uint64_t phys, uint64_t len, uint32_t *flags)
{
	void	*p;

	*flags = VM_WC;
	if (!cpu_features()->pat)
		*flags = VM_NOCACHE;
	p = vmm_io_map(phys, len, *flags);
	if (!p && *flags == VM_WC)
	{
		*flags = VM_NOCACHE;
		p = vmm_io_map(phys, len, *flags);
	}
	return (p);
}

void	display_surface_set(t_surface *s, void *px, const t_dispinfo *i)
{
	s->px = px;
	s->w = (int32_t)i->width;
	s->h = (int32_t)i->height;
	s->stride = (int32_t)(i->pitch / (DISP_BPP / 8));
	s->clip = rect_make(0, 0, s->w, s->h);
}

static void	fill(const t_dispinfo *info, uint64_t phys, void *px)
{
	g_display.info = *info;
	if (g_display.map_flags == VM_WC)
		g_display.info.flags |= DISP_FLAG_WC;
	g_display.phys = phys;
	g_display.map_len = align_up(info->fb_size, PAGE_SIZE);
	g_display.fb = px;
	display_surface_set(&g_display.surf, px, &g_display.info);
	g_display.dispi.boot_w = info->width;
	g_display.dispi.boot_h = info->height;
	g_display.ready = true;
}

int	display_open(void)
{
	const t_fbinfo	*fb;
	t_dispinfo		info;
	void			*px;
	int				rc;

	fb = &boot_info()->fb;
	if (!fb->width && !fb->height)
		return (E_NODEV);
	rc = fbgeom_check(fb, &info);
	if (rc < 0)
		return (rc);
	px = map_fb(fb->phys, align_up(info.fb_size, PAGE_SIZE),
			&g_display.map_flags);
	if (!px)
		return (E_NOMEM);
	fill(&info, fb->phys, px);
	return (E_OK);
}
