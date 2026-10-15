#include "display_int.h"
#include "../../kernel/kcon/kcon_int.h"
#include "velum/klog.h"
#include "velum/panic.h"

static int	check_info(void)
{
	const t_fbinfo		*fb;
	const t_dispinfo	*i;

	fb = &boot_info()->fb;
	i = display_info();
	if (i->width != fb->width || i->height != fb->height)
		return (1);
	if (i->pitch < i->width * (DISP_BPP / 8) || i->bpp != DISP_BPP)
		return (1);
	if (i->fb_size < (uint64_t)i->pitch * i->height)
		return (1);
	if (!display_fb() || display_fb_phys() != fb->phys)
		return (1);
	return (0);
}

static int	check_wrap(void)
{
	t_bsod_wrap	w;

	w.text = "aaa bbb ccc ddd eee";
	w.cols = 7;
	w.max = 2;
	bsod_wrap(&w);
	if (w.count != 2 || w.lines[0].len != 7 || !w.truncated)
		return (1);
	return (w.lines[1].len != 4);
}

static int	check_libs(void)
{
	int	bad;

	bad = (gfx_selftest() != 0);
	if (bad)
		klog_err("display: gfx_selftest en echec");
	if (font_selftest() != 0)
	{
		klog_err("display: font_selftest en echec");
		bad++;
	}
	return (bad);
}

int	display_selftest(void)
{
	int	bad;

	bad = check_libs();
	if (!display_ready())
	{
		klog_info("display: pas de framebuffer, essai ignoré");
		return (bad);
	}
	bad += check_info();
	bad += selftest_text();
	bad += check_wrap();
	bad += selftest_modes();
	if (bad)
		klog_err("display: %d anomalie(s)", bad);
	if (!bad && display_fault_requested())
		panic("a13: panne volontaire (fault=panic)");
	return (bad);
}
