#include <stdio.h>
#include "harness.h"
#include "lt_bbox.h"

static const int32_t	g_dim[][2] = {{1280, 800}, {1024, 768}, {640, 480},
	{801, 601}, {320, 200}, {200, 400}};
static const uint32_t	g_pct[3] = {0, 50, 100};

static t_rect	boot_zone(t_lt *back, t_lt *front, uint32_t pct)
{
	t_rect	all;

	all = lp_rect(0, 0, back->w, back->h);
	luna_logon_bg(&back->s, all);
	luna_boot_draw(&front->s, pct, 7);
	return (ltb_bbox(&front->s, &back->s, 0));
}

static void	zone_check(t_rect z, int32_t w, int32_t h, uint32_t pct)
{
	char	name[96];

	snprintf(name, sizeof(name), "zone non vide, %dx%d, pct %u",
		(int)w, (int)h, (unsigned)pct);
	h_true(z.w > 0 && z.h > 0, name);
	snprintf(name, sizeof(name), "carte centree en x, %dx%d, pct %u",
		(int)w, (int)h, (unsigned)pct);
	h_true(z.x + z.w / 2 >= w / 2 - 2 && z.x + z.w / 2 <= w / 2 + 2, name);
	snprintf(name, sizeof(name), "carte centree en y, %dx%d, pct %u",
		(int)w, (int)h, (unsigned)pct);
	h_true(z.y + z.h / 2 >= h / 2 - 2 && z.y + z.h / 2 <= h / 2 + 2, name);
	snprintf(name, sizeof(name), "carte dans la surface, %dx%d", (int)w,
		(int)h);
	h_true(ltb_inside(z, lp_rect(0, 0, w, h)), name);
}

static void	boot_size(int32_t w, int32_t h, bool dark)
{
	t_lt	back;
	t_lt	front;
	int		i;

	luna_theme_set(LUNA_THEME_LIGHT);
	if (dark)
		luna_theme_set(LUNA_THEME_DARK);
	luna_transparency_set(true);
	h_true(lt_open(&back, w, h) == 0 && lt_open(&front, w, h) == 0,
		"allocation");
	i = 0;
	while (i < 3)
	{
		zone_check(boot_zone(&back, &front, g_pct[i]), w, h, g_pct[i]);
		i++;
	}
	h_true(lt_guard_ok(&front), "zones de garde intactes");
	lt_close(&front);
	lt_close(&back);
}

static void	boot_partition(void)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_dim) / sizeof(g_dim[0]))
	{
		boot_size(g_dim[i][0], g_dim[i][1], false);
		boot_size(g_dim[i][0], g_dim[i][1], true);
		i++;
	}
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
}

int	main(void)
{
	h_begin("a17/boot_layout");
	h_run("P8 ecran de demarrage centre, partitions de tailles, deux themes",
		boot_partition);
	return (h_end());
}
