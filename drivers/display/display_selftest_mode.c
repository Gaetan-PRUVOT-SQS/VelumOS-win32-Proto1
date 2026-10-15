#include "display_int.h"
#include "velum/boot.h"
#include "velum/klog.h"

static bool	parse_mode(const char *v, uint32_t *w, uint32_t *h)
{
	*w = 0;
	*h = 0;
	while (v && *v >= '0' && *v <= '9' && *w < 100000)
		*w = *w * 10 + (uint32_t)(*v++ - '0');
	if (!v || *v != 'x')
		return (false);
	v++;
	while (*v >= '0' && *v <= '9' && *h < 100000)
		*h = *h * 10 + (uint32_t)(*v++ - '0');
	return (*w > 0 && *h > 0 && *v == '\0');
}

static int	check_pixel(void)
{
	t_surface	*s;
	t_point		p;

	s = display_surface();
	p.x = s->w - 1;
	p.y = s->h - 1;
	gfx_put(s, p, 0xff00ff00);
	return (gfx_get(s, p) != 0xff00ff00);
}

static int	roundtrip(const t_dispmode *m, uint32_t n)
{
	uint32_t	w;
	uint32_t	h;
	uint32_t	i;
	int			bad;

	i = 0;
	while (i < n && (m[i].flags & DISP_MODE_CURRENT))
		i++;
	if (i >= n)
		return (0);
	w = m[i].width;
	h = m[i].height;
	bad = (display_set_mode(w, h, DISP_BPP) < 0);
	bad += (display_info()->width != w || display_info()->height != h);
	bad += (display_info()->pitch != w * (DISP_BPP / 8));
	bad += check_pixel();
	return (bad);
}

static int	finish(void)
{
	const char	*want;
	uint32_t	w;
	uint32_t	h;

	want = boot_cmdline_get("dispmode");
	w = g_display.dispi.boot_w;
	h = g_display.dispi.boot_h;
	if (want && !parse_mode(want, &w, &h))
		return (1);
	return (display_set_mode(w, h, DISP_BPP) < 0);
}

int	selftest_modes(void)
{
	t_dispmode	m[DISP_MODES_MAX];
	uint32_t	n;
	uint32_t	cur;
	uint32_t	i;
	int			bad;

	n = display_modes(m, DISP_MODES_MAX);
	cur = 0;
	i = 0;
	while (i < n)
		cur += (m[i++].flags & DISP_MODE_CURRENT) != 0;
	bad = (n == 0 || cur != 1);
	if (!g_display.dispi.ready)
		return (bad);
	bad += roundtrip(m, n);
	bad += finish();
	klog_info("display: essai de modes, %d anomalie(s)", bad);
	return (bad);
}
