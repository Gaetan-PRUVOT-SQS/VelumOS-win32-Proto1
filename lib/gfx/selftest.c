#include "velum/err.h"
#include "gfx_int.h"

static uint32_t	g_selftest_px[64];

static int	check_blend(void)
{
	if (gfx_blend(0xff000000, 0x80ffffff) != 0xff808080)
		return (E_INVAL);
	if (gfx_blend(0xff102030, 0x00ffffff) != 0xff102030)
		return (E_INVAL);
	if (gfx_blend(0xff102030, 0xffabcdef) != 0xffabcdef)
		return (E_INVAL);
	if (gfx_lerp(0xff000000, 0xffffffff, 256) != 0xffffffff)
		return (E_INVAL);
	return (E_OK);
}

static int	check_fill(t_surface *s)
{
	memset(g_selftest_px, 0, sizeof(g_selftest_px));
	gfx_fill(s, rect_make(2, 2, 4, 4), 0xffff0000);
	if (gfx_get(s, (t_point){2, 2}) != 0xffff0000)
		return (E_INVAL);
	if (gfx_get(s, (t_point){5, 5}) != 0xffff0000)
		return (E_INVAL);
	if (gfx_get(s, (t_point){6, 6}) != 0 || gfx_get(s, (t_point){1, 2}) != 0)
		return (E_INVAL);
	gfx_fill(s, rect_make(-100, -100, 1000, 1000), 0xff00ff00);
	if (gfx_get(s, (t_point){7, 7}) != 0xff00ff00)
		return (E_INVAL);
	return (E_OK);
}

static int	check_line(t_surface *s)
{
	t_point	p;

	memset(g_selftest_px, 0, sizeof(g_selftest_px));
	gfx_line(s, (t_point){0, 0}, (t_point){7, 7}, 0xffffffff);
	p.x = 0;
	while (p.x < 8)
	{
		p.y = p.x;
		if (gfx_get(s, p) != 0xffffffff)
			return (E_INVAL);
		p.x++;
	}
	if (gfx_get(s, (t_point){1, 0}) != 0)
		return (E_INVAL);
	return (E_OK);
}

static int	check_region(void)
{
	t_region	rg;
	t_rect		b;

	region_clear(&rg);
	region_add(&rg, rect_make(0, 0, 4, 4));
	region_add(&rg, rect_make(4, 0, 4, 4));
	b = region_bounds(&rg);
	if (rg.n != 1 || b.x != 0 || b.y != 0 || b.w != 8 || b.h != 4)
		return (E_INVAL);
	region_subtract(&rg, rect_make(0, 0, 8, 4));
	if (!region_empty(&rg))
		return (E_INVAL);
	return (E_OK);
}

int	gfx_selftest(void)
{
	t_surface	s;
	int			rc;

	gfx_surface_init(&s, g_selftest_px, 8, 8);
	rc = check_blend();
	if (rc == E_OK)
		rc = check_fill(&s);
	if (rc == E_OK)
		rc = check_line(&s);
	if (rc == E_OK)
		rc = check_region();
	return (rc);
}
