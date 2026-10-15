#include "display_int.h"

static const t_dispmode	g_table[] = {
{800, 600, DISP_BPP, 0},
{1024, 768, DISP_BPP, 0},
{1280, 720, DISP_BPP, 0},
{1280, 1024, DISP_BPP, 0},
{1920, 1080, DISP_BPP, 0}
};

static bool	fits(const t_dispi *d, uint32_t w, uint32_t h)
{
	if (!w || !h || w > UINT16_MAX || h > UINT16_MAX)
		return (false);
	return ((uint64_t)w * h * DISPI_BYTES_PIXEL <= d->vram);
}

static bool	listed(uint32_t w, uint32_t h)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_table) / sizeof(g_table[0]))
	{
		if (g_table[i].width == w && g_table[i].height == h)
			return (true);
		i++;
	}
	return (false);
}

bool	dispi_mode_ok(const t_dispi *d, uint32_t w, uint32_t h)
{
	bool	known;

	known = listed(w, h) || (w == d->boot_w && h == d->boot_h);
	return (known && fits(d, w, h));
}

static void	put_mode(t_modelist *l, uint32_t w, uint32_t h)
{
	l->total++;
	if (!l->out || l->written >= l->max)
		return ;
	l->out[l->written].width = w;
	l->out[l->written].height = h;
	l->out[l->written].bpp = DISP_BPP;
	l->out[l->written].flags = 0;
	if (w == l->cur_w && h == l->cur_h)
		l->out[l->written].flags = DISP_MODE_CURRENT;
	l->written++;
}

uint32_t	dispi_list(const t_dispi *d, t_modelist *l)
{
	uint32_t	i;

	l->total = 0;
	l->written = 0;
	i = 0;
	while (i < sizeof(g_table) / sizeof(g_table[0]))
	{
		if (fits(d, g_table[i].width, g_table[i].height))
			put_mode(l, g_table[i].width, g_table[i].height);
		i++;
	}
	if (!listed(d->boot_w, d->boot_h) && fits(d, d->boot_w, d->boot_h))
		put_mode(l, d->boot_w, d->boot_h);
	if (l->max == 0)
		return (l->total);
	return (l->written);
}
