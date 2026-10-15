#include "../../kernel/kcon/kcon_int.h"
#include "display_int.h"
#include "velum/libk.h"

static uint32_t	g_st_px[ST_W * ST_H];

static bool	has_fg(const t_kcon *k, int32_t col, int32_t row)
{
	int32_t	x;
	int32_t	y;

	y = row * k->cell_h;
	while (y < (row + 1) * k->cell_h)
	{
		x = col * k->cell_w;
		while (x < (col + 1) * k->cell_w)
		{
			if (g_st_px[y * ST_W + x] == k->fg)
				return (true);
			x++;
		}
		y++;
	}
	return (false);
}

static int	check_write(t_kcon *k)
{
	int	bad;

	bad = 0;
	kcon_feed(k, "AB", 2);
	bad += !(k->col == 2 && k->row == 0);
	bad += !has_fg(k, 0, 0);
	kcon_feed(k, "\b\b\b", 3);
	bad += (k->col != 0);
	kcon_feed(k, "\n\n\n\n\n\n", 6);
	bad += (k->row != k->rows - 1);
	return (bad);
}

static int	check_wrap(t_kcon *k)
{
	int32_t	i;
	int		bad;

	kcon_clear_all(k);
	i = 0;
	while (i < k->cols)
	{
		kcon_feed(k, "x", 1);
		i++;
	}
	bad = !(k->col == k->cols && k->row == 0);
	kcon_feed(k, "y", 1);
	bad += !(k->col == 1 && k->row == 1);
	return (bad);
}

int	selftest_text(void)
{
	t_surface	s;
	t_kcon		k;

	memset(g_st_px, 0, sizeof(g_st_px));
	s.px = g_st_px;
	s.w = ST_W;
	s.h = ST_H;
	s.stride = ST_W;
	s.clip = rect_make(0, 0, ST_W, ST_H);
	if (kcon_setup(&k, &s, font_get(FONT_MONO)) < 0)
		return (1);
	return (check_write(&k) + check_wrap(&k));
}
