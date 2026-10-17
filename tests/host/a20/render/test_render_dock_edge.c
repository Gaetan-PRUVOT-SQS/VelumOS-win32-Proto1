#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include "harness.h"
#include "dock_int.h"
#include "render_edge.h"

static void	valeurs_limites_du_rectangle(void)
{
	t_edge		e;
	t_ldplate	p;

	h_true(edge_open(&e), "memoire de test");
	p = (t_ldplate){rect_make(1, 0, INT_MAX, EDGE_H), 0, EDGE_INK, 0};
	ld_plate(&e.s, &p);
	h_eq_u64("largeur INT_MAX: colonne 0 intacte", gfx_get(&e.s,
			(t_point){0, 8}), EDGE_BG);
	h_eq_u64("largeur INT_MAX: pixel plein", gfx_get(&e.s, (t_point){10, 8}),
		EDGE_INK);
	p.r = rect_make(INT_MIN, 0, 20, EDGE_H);
	p.c = 0xffffffffu;
	ld_plate(&e.s, &p);
	p.r = rect_make(2, 2, -5, 4);
	ld_plate(&e.s, &p);
	p.r = rect_make(INT_MAX, INT_MAX, 5, 5);
	ld_plate(&e.s, &p);
	h_true(gfx_get(&e.s, (t_point){5, 5}) == EDGE_INK, "rects hors bornes");
	h_true(edge_guard_ok(&e), "zones de garde intactes");
	free(e.all);
}

static void	valeurs_limites_de_l_epaisseur(void)
{
	t_edge		e;
	t_ldplate	p;

	h_true(edge_open(&e), "memoire de test");
	p = (t_ldplate){rect_make(2, 2, 20, 10), 0, EDGE_INK, INT_MAX};
	ld_ring(&e.s, &p);
	h_eq_u64("epaisseur INT_MAX: bornee, anneau plein",
		gfx_get(&e.s, (t_point){12, 7}), EDGE_INK);
	p.thick = INT_MIN;
	p.c = 0xffffffffu;
	ld_ring(&e.s, &p);
	h_eq_u64("epaisseur INT_MIN: rien", gfx_get(&e.s, (t_point){12, 7}),
		EDGE_INK);
	p.thick = 1;
	p.r = rect_make(0, 0, EDGE_W, EDGE_H);
	p.c = EDGE_BG;
	ld_ring(&e.s, &p);
	h_true(edge_guard_ok(&e), "zones de garde intactes");
	free(e.all);
}

static void	valeurs_limites_du_rayon(void)
{
	t_edge		e;
	t_ldplate	p;

	h_true(edge_open(&e), "memoire de test");
	p = (t_ldplate){rect_make(0, 0, EDGE_W, EDGE_H), INT_MAX, EDGE_INK, 0};
	ld_plate(&e.s, &p);
	h_eq_u64("rayon INT_MAX: coin arrondi", gfx_get(&e.s, (t_point){0, 0}),
		EDGE_BG);
	h_eq_u64("rayon INT_MAX: centre plein", gfx_get(&e.s, (t_point){16, 8}),
		EDGE_INK);
	p.rad = INT_MIN;
	p.thick = 3;
	ld_ring(&e.s, &p);
	ld_rim(&e.s, &p);
	h_true(edge_guard_ok(&e), "zones de garde intactes");
	free(e.all);
}

int	main(void)
{
	h_begin("a20/render-dock-edge");
	h_run("dock: valeurs limites du rectangle", valeurs_limites_du_rectangle);
	h_run("dock: valeurs limites de l'epaisseur",
		valeurs_limites_de_l_epaisseur);
	h_run("dock: valeurs limites du rayon", valeurs_limites_du_rayon);
	return (h_end());
}
