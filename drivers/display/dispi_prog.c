#include "display_int.h"
#include "velum/err.h"

static int	verify(t_dispi *d, uint32_t w, uint32_t h, t_dispi_geom *out)
{
	uint32_t	virt;

	if (dispi_rd(d, DISPI_INDEX_XRES) != w)
		return (E_IO);
	if (dispi_rd(d, DISPI_INDEX_YRES) != h)
		return (E_IO);
	if (dispi_rd(d, DISPI_INDEX_BPP) != DISP_BPP)
		return (E_IO);
	if ((dispi_rd(d, DISPI_INDEX_ENABLE) & DISPI_ON) != DISPI_ON)
		return (E_IO);
	virt = dispi_rd(d, DISPI_INDEX_VIRT_WIDTH);
	if (virt < w)
		virt = w;
	out->width = w;
	out->height = h;
	out->pitch = virt * DISPI_BYTES_PIXEL;
	if ((uint64_t)out->pitch * h > d->vram)
		return (E_NOSPC);
	return (E_OK);
}

static int	apply(t_dispi *d, uint32_t w, uint32_t h, t_dispi_geom *out)
{
	dispi_wr(d, DISPI_INDEX_ENABLE, DISPI_OFF);
	dispi_wr(d, DISPI_INDEX_XRES, (uint16_t)w);
	dispi_wr(d, DISPI_INDEX_YRES, (uint16_t)h);
	dispi_wr(d, DISPI_INDEX_BPP, DISP_BPP);
	dispi_wr(d, DISPI_INDEX_ENABLE, DISPI_ON);
	return (verify(d, w, h, out));
}

int	dispi_program(t_dispi *d, uint32_t w, uint32_t h, t_dispi_geom *out)
{
	t_dispi_geom	prev;
	int				rc;

	if (!d->ready)
		return (E_NODEV);
	if (!dispi_mode_ok(d, w, h))
		return (E_INVAL);
	prev.width = dispi_rd(d, DISPI_INDEX_XRES);
	prev.height = dispi_rd(d, DISPI_INDEX_YRES);
	rc = apply(d, w, h, out);
	if (rc < 0 && prev.width && prev.height)
		apply(d, prev.width, prev.height, &prev);
	return (rc);
}
