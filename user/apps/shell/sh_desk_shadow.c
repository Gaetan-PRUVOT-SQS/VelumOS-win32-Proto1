#include <stdlib.h>
#include "../common/uicolor.h"
#include "sh_desk.h"

static void	icon_on(const t_shadowjob *j, uint32_t *px, t_color bg)
{
	t_surface	tile;

	gfx_surface_init(&tile, px, j->n, j->n);
	gfx_fill(&tile, rect_make(0, 0, j->n, j->n), bg);
	luna_icon(&tile, (t_point){SH_SHADOW_PAD, SH_SHADOW_PAD}, j->id, j->size);
}

static void	extract(const t_shadowjob *j)
{
	const uint32_t	*black;
	const uint32_t	*white;
	int32_t			i;
	int32_t			diff;

	black = j->px;
	white = j->px + j->n * j->n;
	i = 0;
	while (i < j->n * j->n)
	{
		diff = (int32_t)((white[i] >> 8) & 0xffu)
			- (int32_t)((black[i] >> 8) & 0xffu);
		if (diff < 0)
			diff = 0;
		if (diff > 255)
			diff = 255;
		j->a[i] = (uint8_t)(255 - diff);
		i++;
	}
}

static void	composite(const t_shadowjob *j, t_surface *dst, t_point o)
{
	int32_t		x;
	int32_t		y;
	uint32_t	cover;

	y = 0;
	while (y < j->n)
	{
		x = 0;
		while (x < j->n)
		{
			cover = (uint32_t)j->a[y * j->n + x] * SH_SHADOW_A / 255;
			gfx_put(dst, (t_point){o.x + x, o.y + y},
				(cover << 24) | SH_SHADOW_RGB);
			x++;
		}
		y++;
	}
}

void	sh_icon_shadow(t_surface *s, t_point p, t_iconid id, int32_t size)
{
	t_shadowjob	j;
	size_t		count;

	if (!s || size < 1 || size > SH_SHADOW_MAX)
		return ;
	j.n = size + 2 * SH_SHADOW_PAD;
	j.size = size;
	j.id = id;
	count = (size_t)j.n * (size_t)j.n;
	j.px = malloc(count * (2 * sizeof(uint32_t) + 2));
	if (!j.px)
		return ;
	j.a = (uint8_t *)(j.px + 2 * count);
	j.tmp = j.a + count;
	icon_on(&j, j.px, UI_BLACK);
	icon_on(&j, j.px + count, UI_WHITE);
	extract(&j);
	sh_blur_alpha(j.a, j.tmp, j.n);
	composite(&j, s, (t_point){p.x - SH_SHADOW_PAD,
		p.y - SH_SHADOW_PAD + SH_SHADOW_DY});
	free(j.px);
}
