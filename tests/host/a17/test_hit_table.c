#include <stdio.h>
#include "harness.h"
#include "lt_hit.h"

static void	hit_one(const t_lthit *c)
{
	t_lunawin	w;
	uint32_t	got;
	char		msg[96];

	w = lt_win(c->outer, c->style, c->maximized);
	got = luna_hit_test(&w, c->p);
	snprintf(msg, sizeof(msg), "%s (%d,%d) attendu %u obtenu %u", c->name,
		c->p.x, c->p.y, c->want, got);
	h_true(got == c->want, msg);
}

static void	hit_table(void)
{
	int	i;

	i = 0;
	while (i < g_hit_count)
	{
		hit_one(&g_hit_cases[i]);
		i++;
	}
}

static void	hit_outside_extremes(void)
{
	t_lunawin	w;

	w = lt_win(lp_rect(INT32_MAX - 10, INT32_MAX - 10, 100, 100), WS_DEFAULT,
			false);
	h_eq_u64("loin a droite", luna_hit_test(&w, lp_pt(INT32_MAX, INT32_MAX)),
		HT_NOWHERE);
	w = lt_win(lp_rect(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX), WS_DEFAULT,
			false);
	h_eq_u64("loin a gauche", luna_hit_test(&w, lp_pt(INT32_MIN, 0)),
		HT_NOWHERE);
	w = lt_win(lp_rect(0, 0, -5, -5), WS_DEFAULT, false);
	h_eq_u64("taille negative", luna_hit_test(&w, lp_pt(0, 0)), HT_NOWHERE);
	w = lt_win(lp_rect(0, 0, 0, 0), WS_DEFAULT, false);
	h_eq_u64("taille nulle", luna_hit_test(&w, lp_pt(0, 0)), HT_NOWHERE);
	w = lt_win(lp_rect(0, 0, 100, 100), WS_DEFAULT, false);
	h_eq_u64("point INT32_MIN", luna_hit_test(&w, lp_pt(INT32_MIN, INT32_MIN)),
		HT_NOWHERE);
	h_eq_u64("point INT32_MAX", luna_hit_test(&w, lp_pt(INT32_MAX, 5)),
		HT_NOWHERE);
}

int	main(void)
{
	h_begin("a17/hit_table");
	h_run("table de decision des zones", hit_table);
	h_run("coordonnees extremes", hit_outside_extremes);
	return (h_end());
}
