#include "../../kernel/kcon/kcon_int.h"
#include "display_int.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/luna.h"
#include "velum/util.h"
#include "velum/vmm.h"

static int	remap(const t_dispi_geom *g, t_dispinfo *ni, void **px)
{
	uint64_t	len;

	ni->width = g->width;
	ni->height = g->height;
	ni->pitch = g->pitch;
	ni->bpp = DISP_BPP;
	ni->format = DISP_FMT_XRGB8888;
	ni->flags = g_display.info.flags;
	ni->fb_size = (uint64_t)g->pitch * g->height;
	len = align_up(ni->fb_size, PAGE_SIZE);
	*px = g_display.fb;
	if (len <= g_display.map_len)
		return (E_OK);
	*px = vmm_io_map(g_display.phys, len, g_display.map_flags);
	if (!*px)
		return (E_NOMEM);
	return (E_OK);
}

static void	redraw(void)
{
	if (g_display.state == DSP_SPLASH)
		luna_boot_draw(&g_display.surf, g_display.pct, g_display.tick);
	else if (g_display.state == DSP_CONSOLE)
	{
		if (g_kcon.ready)
			kcon_clear_all(&g_kcon);
		else
			g_display.state = DSP_OFF;
	}
}

static void	commit(const t_dispinfo *ni, void *px)
{
	void		*old;
	uint64_t	old_len;
	bool		held;

	old = g_display.fb;
	old_len = g_display.map_len;
	held = display_lock_wait();
	g_display.info = *ni;
	if (px != old)
	{
		g_display.fb = px;
		g_display.map_len = align_up(ni->fb_size, PAGE_SIZE);
	}
	display_surface_set(&g_display.surf, px, &g_display.info);
	g_display.generation++;
	kcon_attach(&g_display.surf);
	redraw();
	if (held)
		display_unlock();
	if (px != old && held)
		vmm_io_unmap(old, old_len);
}

static void	restore(void)
{
	t_dispi_geom	geom;

	dispi_program(&g_display.dispi, g_display.info.width,
		g_display.info.height, &geom);
}

int	display_switch(uint32_t w, uint32_t h)
{
	t_dispi_geom	geom;
	t_dispinfo		ni;
	void			*px;
	int				rc;

	rc = dispi_program(&g_display.dispi, w, h, &geom);
	if (rc < 0)
		return (rc);
	rc = remap(&geom, &ni, &px);
	if (rc < 0)
	{
		restore();
		return (rc);
	}
	commit(&ni, px);
	klog_info("display: mode %ux%u", w, h);
	return (E_OK);
}
