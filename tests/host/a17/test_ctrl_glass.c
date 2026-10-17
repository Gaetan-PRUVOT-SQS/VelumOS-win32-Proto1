#include "harness.h"
#include "lt.h"
#include "luna_ctrl.h"

static t_lt	g_t;

static void	sym_row(const t_lrfill *o, const t_lrfill *in, int32_t y)
{
	t_point	p;
	int32_t	c;

	p.y = y;
	p.x = 0;
	while (p.x < o->r.w)
	{
		c = lp_rcov(o, p);
		h_true(c == lp_rcov(o, lp_pt(o->r.w - 1 - p.x, y)),
			"symetrie horizontale");
		h_true(c == lp_rcov(o, lp_pt(p.x, o->r.h - 1 - y)),
			"symetrie verticale");
		h_true(lp_rcov(in, p) <= c, "la forme interieure est incluse");
		p.x++;
	}
}

static void	partition_couverture(void)
{
	t_lrfill	o;
	t_lrfill	in;
	int32_t		y;

	o = lp_rmake(lp_rect(0, 0, 20, 14), 6, 0, 0);
	in = lp_rmake(lp_inset(o.r, 1), 5, 0, 0);
	h_eq_i64("coin exterieur vide", lp_rcov(&o, lp_pt(0, 0)), 0);
	h_eq_i64("centre plein", lp_rcov(&o, lp_pt(10, 7)), 16);
	h_eq_i64("milieu du bord haut plein", lp_rcov(&o, lp_pt(10, 0)), 16);
	h_eq_i64("hors forme", lp_rcov(&o, lp_pt(-1, 5)), 0);
	y = 0;
	while (y < o.r.h)
	{
		sym_row(&o, &in, y);
		y++;
	}
}

static void	un_seul_empilement(void)
{
	t_lrfill	f;

	h_true(lt_open(&g_t, 40, 20) == 0, "allocation");
	lt_clear(&g_t, LC_BLACK);
	f = lp_rmake(lp_rect(4, 4, 32, 12), 4, 0x80ffffff, 0x80ffffff);
	lp_rfill(&g_t.s, &f);
	h_eq_u64("une seule couche a 50 %", gfx_get(&g_t.s, lp_pt(20, 10)),
		0xff808080U);
	h_eq_u64("hors forme intact", gfx_get(&g_t.s, lp_pt(2, 2)), 0xff000000U);
	lt_close(&g_t);
}

static void	anneau_exclusif(void)
{
	t_lrfill	edge;
	t_lrfill	in;

	h_true(lt_open(&g_t, 40, 20) == 0, "allocation");
	lt_clear(&g_t, LC_BLACK);
	edge = lp_rmake(lp_rect(4, 4, 32, 12), 4, 0x80ffffff, 0x80ffffff);
	in = lp_rmake(lp_inset(edge.r, 1), 3, 0x80ffffff, 0x80ffffff);
	lp_rfill(&g_t.s, &in);
	lp_rring(&g_t.s, &edge, 1);
	h_eq_u64("anneau a 50 % sans cumul", gfx_get(&g_t.s, lp_pt(20, 4)),
		0xff808080U);
	h_eq_u64("interieur a 50 %", gfx_get(&g_t.s, lp_pt(20, 8)), 0xff808080U);
	h_eq_u64("pas de debordement", gfx_get(&g_t.s, lp_pt(20, 3)), 0xff000000U);
	lt_close(&g_t);
}

int	main(void)
{
	h_begin("a17/ctrl_verre");
	h_run("partition : couverture, symetrie et inclusion",
		partition_couverture);
	h_run("metamorphique : un remplissage translucide ne s'empile pas",
		un_seul_empilement);
	h_run("metamorphique : l'anneau ne recouvre pas le remplissage",
		anneau_exclusif);
	return (h_end());
}
