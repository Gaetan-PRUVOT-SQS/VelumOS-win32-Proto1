#include "display_int.h"
#include "velum/err.h"
#include "velum/util.h"

static int	dims_ok(const t_fbinfo *fb)
{
	if (fb->width < 1 || fb->width > DISP_DIM_MAX)
		return (E_RANGE);
	if (fb->height < 1 || fb->height > DISP_DIM_MAX)
		return (E_RANGE);
	if (fb->bpp != DISP_BPP)
		return (E_NOTSUP);
	return (E_OK);
}

static int	format_ok(const t_fbinfo *fb)
{
	if (fb->red_shift != 16 || fb->red_size != 8)
		return (E_NOTSUP);
	if (fb->green_shift != 8 || fb->green_size != 8)
		return (E_NOTSUP);
	if (fb->blue_shift != 0 || fb->blue_size != 8)
		return (E_NOTSUP);
	return (E_OK);
}

static int	pitch_ok(const t_fbinfo *fb)
{
	uint64_t	min;

	min = (uint64_t)fb->width * (DISP_BPP / 8);
	if (fb->pitch < min || fb->pitch > DISP_PITCH_MAX)
		return (E_RANGE);
	if (fb->pitch % (DISP_BPP / 8))
		return (E_INVAL);
	if ((uint64_t)fb->pitch * fb->height > DISP_BYTES_MAX)
		return (E_RANGE);
	return (E_OK);
}

static int	phys_ok(const t_fbinfo *fb)
{
	uint64_t	len;

	len = align_up((uint64_t)fb->pitch * fb->height, PAGE_SIZE);
	if (!fb->phys || !is_aligned(fb->phys, PAGE_SIZE))
		return (E_INVAL);
	if (fb->phys >= DISP_PHYS_LIMIT || len > DISP_PHYS_LIMIT - fb->phys)
		return (E_OVERFLOW);
	return (E_OK);
}

int	fbgeom_check(const t_fbinfo *fb, t_dispinfo *out)
{
	int	rc;

	rc = dims_ok(fb);
	if (rc == E_OK)
		rc = format_ok(fb);
	if (rc == E_OK)
		rc = pitch_ok(fb);
	if (rc == E_OK)
		rc = phys_ok(fb);
	if (rc < 0)
		return (rc);
	out->width = fb->width;
	out->height = fb->height;
	out->pitch = fb->pitch;
	out->bpp = fb->bpp;
	out->format = DISP_FMT_XRGB8888;
	out->flags = 0;
	out->fb_size = (uint64_t)fb->pitch * fb->height;
	return (E_OK);
}
