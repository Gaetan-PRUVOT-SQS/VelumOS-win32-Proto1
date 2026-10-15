#include "harness.h"
#include "fake.h"
#include "fake_sb.h"

static const int	g_bw[6] = {17, 100, 17, 17, 17, 17};
static const int	g_bh[6] = {100, 17, 20, 34, 10, 1};

static void	layout_partitions(void)
{
	t_scrst	st;
	int		i;
	int		max;

	i = 0;
	while (i < 6)
	{
		max = 0;
		while (max <= 90)
		{
			st.max = max;
			st.page = 10;
			st.pos = max / 2;
			st.zone = 0;
			st.grab = 0;
			fake_check_layout(rect_make(5, 7, g_bw[i], g_bh[i]), st);
			max += 30;
		}
		i++;
	}
}

static void	thumb_ends(void)
{
	t_scrst	st;
	t_sblo	lo;
	t_rect	bar;

	bar = rect_make(0, 0, 17, 100);
	st = (t_scrst){0, 90, 10, 0, 0};
	ctl_sb_layout(bar, &st, &lo);
	h_eq_i64("pos 0 : pouce au debut de la piste", lo.thumb.y, lo.track.y);
	h_eq_i64("longueur minimale 8", lo.thumb.h, CTL_THUMB_MIN);
	st.pos = 90;
	ctl_sb_layout(bar, &st, &lo);
	h_eq_i64("pos max : pouce a la fin", lo.thumb.y + lo.thumb.h,
		lo.track.y + lo.track.h);
	st = (t_scrst){0, 0, 10, 0, 0};
	ctl_sb_layout(bar, &st, &lo);
	h_true(rect_empty(lo.thumb), "rien a defiler : pas de pouce");
	st = (t_scrst){0, 1, 100, 0, 0};
	ctl_sb_layout(bar, &st, &lo);
	h_true(lo.thumb.h > lo.track.h * 9 / 10, "grande page : grand pouce");
	h_true(ctl_sb_vertical(rect_make(0, 0, 17, 17)), "carre : vertical");
	h_true(!ctl_sb_vertical(rect_make(0, 0, 100, 17)), "large : horizontal");
}

static void	zones_partition(void)
{
	t_scrst	st;
	t_sblo	lo;
	t_rect	bar;
	int		counts[6];

	bar = rect_make(5, 7, 17, 100);
	st = (t_scrst){40, 90, 10, 0, 0};
	ctl_sb_layout(bar, &st, &lo);
	fake_zone_counts(&lo, counts);
	h_eq_i64("dec : 17x17", counts[CTL_SBZ_DEC], 17 * 17);
	h_eq_i64("inc : 17x17", counts[CTL_SBZ_INC], 17 * 17);
	h_eq_i64("pouce", counts[CTL_SBZ_THUMB], 17 * CTL_THUMB_MIN);
	h_eq_i64("hors barre", counts[CTL_SBZ_NONE], 30 * 120 - 17 * 100);
	h_eq_i64("pistes = 17 x (66 - 8)", counts[CTL_SBZ_PAGE_DEC]
		+ counts[CTL_SBZ_PAGE_INC], 17 * 58);
}

static void	pos_monotonic(void)
{
	t_scrst	st;
	t_sblo	lo;
	int		coord;
	int32_t	last;
	int32_t	pos;

	st = (t_scrst){0, 90, 10, 0, 0};
	ctl_sb_layout(rect_make(0, 0, 17, 100), &st, &lo);
	last = 0;
	coord = -20;
	while (coord < 140)
	{
		pos = ctl_sb_pos_from(&lo, &st, coord);
		h_true(pos >= last && pos >= 0 && pos <= 90, "monotone et bornee");
		last = pos;
		coord++;
	}
	h_eq_i64("au bout", last, 90);
	h_true(ctl_sb_set(&st, 5) && !ctl_sb_set(&st, 5), "set : changement");
	h_true(ctl_sb_set(&st, -9) && st.pos == 0, "set : plancher");
	h_true(ctl_sb_set(&st, 999) && st.pos == 90, "set : plafond");
}

int	main(void)
{
	h_begin("a19/scroll_model");
	h_run("decoupage de la barre : 6 tailles x 4 etats", layout_partitions);
	h_run("pouce : extremites, taille mini, orientation", thumb_ends);
	h_run("zones : partition exacte de la barre", zones_partition);
	h_run("position depuis le glisse : monotone", pos_monotonic);
	return (h_end());
}
