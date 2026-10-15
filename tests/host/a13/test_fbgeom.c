#include "cases.h"
#include "fakes.h"
#include "display_int.h"
#include "velum/err.h"

static void	check_info(const t_fbinfo *fb, const t_dispinfo *i, const char *n)
{
	h_eq_u64(n, i->width, fb->width);
	h_eq_u64(n, i->height, fb->height);
	h_eq_u64(n, i->pitch, fb->pitch);
	h_eq_u64(n, i->bpp, 32);
	h_eq_u64(n, i->format, DISP_FMT_XRGB8888);
	h_eq_u64(n, i->flags, 0);
	h_eq_u64(n, i->fb_size, (uint64_t)fb->pitch * fb->height);
}

static void	run_case(const t_geocase *c)
{
	t_fbinfo	fb;
	t_dispinfo	info;
	int			rc;

	g_h.name = c->name;
	fake_fb_set(1024, 768);
	fb = boot_info()->fb;
	fake_fb_field(&fb, c->f1, c->v1);
	fake_fb_field(&fb, c->f2, c->v2);
	memset(&info, 0xaa, sizeof(info));
	rc = fbgeom_check(&fb, &info);
	h_eq_i64(c->name, rc, c->want);
	if (rc == E_OK)
		check_info(&fb, &info, c->name);
}

static void	table_all(void)
{
	unsigned int	i;

	i = 0;
	while (g_geo_cases[i].name)
		run_case(&g_geo_cases[i++]);
}

int	main(void)
{
	h_begin("a13/fbgeom");
	h_run("table limites et MC/DC", table_all);
	return (h_end());
}
