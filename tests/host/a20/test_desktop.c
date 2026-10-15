#include <stdint.h>
#include "harness.h"
#include "velum/abi/abi_input.h"
#include "a20_test.h"
#include "layout.h"
#include "desktop.h"

static void	desktop_grille_ecran_normal(void)
{
	t_rect	out[DESK_ICONS];
	t_rect	area;

	area = lay_rect(0, 0, 1024, 738);
	h_eq_i64("lignes: (738-24-81+99)/99", desk_rows(area), 7);
	h_eq_i64("trois icones", desk_grid(area, DESK_ICONS, out), DESK_ICONS);
	h_eq_i64("premiere: x", out[0].x, 24);
	h_eq_i64("premiere: y", out[0].y, 24);
	h_eq_i64("cellule: largeur", out[0].w, 90);
	h_eq_i64("cellule: hauteur", out[0].h, 81);
	h_eq_i64("seconde: pas vertical de 99", out[1].y, 24 + 99);
	h_eq_i64("meme colonne", out[2].x, 24);
	h_eq_i64("troisieme: y", out[2].y, 24 + 2 * 99);
	h_true(!lay_overlap(out[0], out[1]) && !lay_overlap(out[1], out[2]),
		"cellules disjointes");
	h_true(lay_inside(area, out[2]), "cellules dans la zone");
}

static void	desktop_petites_fenetres(void)
{
	t_rect	out[DESK_ICONS];

	h_eq_i64("une ligne, large", desk_grid(lay_rect(0, 0, 400, 105), 3, out),
		3);
	h_eq_i64("une ligne: 3e en colonne 3", out[2].x, 24 + 2 * 90);
	h_eq_i64("une ligne: meme y", out[2].y, 24);
	h_eq_i64("pile deux colonnes", desk_grid(lay_rect(0, 0, 204, 105), 3,
			out), 2);
	h_eq_i64("un pixel de moins", desk_grid(lay_rect(0, 0, 203, 105), 3, out),
		1);
	h_eq_i64("trop bas pour une ligne", desk_grid(lay_rect(0, 0, 400, 104), 3,
			out), 0);
	h_eq_i64("lignes: 203", desk_rows(lay_rect(0, 0, 400, 203)), 1);
	h_eq_i64("lignes: 204", desk_rows(lay_rect(0, 0, 400, 204)), 2);
	h_eq_i64("aucune icone", desk_grid(lay_rect(0, 0, 400, 400), 0, out), 0);
	h_eq_i64("zone vide", desk_grid(lay_rect(0, 0, 0, 0), 3, out), 0);
	h_eq_i64("zone negative", desk_grid(lay_rect(0, 0, -5, -5), 3, out), 0);
	h_eq_i64("lignes: zone negative", desk_rows(lay_rect(0, 0, -5, -5)), 0);
}

static void	desktop_test_de_clic(void)
{
	t_rect	out[DESK_ICONS];

	desk_grid(lay_rect(0, 0, 1024, 738), DESK_ICONS, out);
	h_eq_i64("coin haut gauche de la 1re", desk_hit(out, 3, 24, 24), 0);
	h_eq_i64("dernier pixel de la 1re", desk_hit(out, 3, 113, 104), 0);
	h_eq_i64("marge gauche", desk_hit(out, 3, 23, 24), DESK_NONE);
	h_eq_i64("marge haute", desk_hit(out, 3, 24, 23), DESK_NONE);
	h_eq_i64("sous la 1re: interstice", desk_hit(out, 3, 24, 105),
		DESK_NONE);
	h_eq_i64("dernier pixel de l'interstice", desk_hit(out, 3, 24, 122),
		DESK_NONE);
	h_eq_i64("premier pixel de la 2e", desk_hit(out, 3, 24, 123), 1);
	h_eq_i64("bureau vide", desk_hit(out, 3, 500, 400), DESK_NONE);
	h_eq_i64("sous la derniere", desk_hit(out, 3, 30, 24 + 2 * 99 + 81),
		DESK_NONE);
	h_eq_i64("aucune cellule", desk_hit(out, 0, 30, 30), DESK_NONE);
}

static void	desktop_clavier(void)
{
	h_eq_i64("bas", desk_move(0, 3, 2, VK_DOWN), 1);
	h_eq_i64("bas: borne", desk_move(2, 3, 2, VK_DOWN), 2);
	h_eq_i64("haut", desk_move(2, 3, 2, VK_UP), 1);
	h_eq_i64("haut: borne", desk_move(0, 3, 2, VK_UP), 0);
	h_eq_i64("droite: colonne suivante", desk_move(0, 3, 2, VK_RIGHT), 2);
	h_eq_i64("droite: borne", desk_move(2, 3, 2, VK_RIGHT), 2);
	h_eq_i64("gauche", desk_move(2, 3, 2, VK_LEFT), 0);
	h_eq_i64("debut et fin", desk_move(1, 3, 2, VK_END) * 10
		+ desk_move(1, 3, 2, VK_HOME), 20);
	h_eq_i64("sans selection: premiere", desk_move(DESK_NONE, 3, 2, VK_DOWN),
		0);
	h_eq_i64("selection hors liste", desk_move(9, 3, 2, VK_UP), 0);
	h_eq_i64("touche inconnue", desk_move(1, 3, 2, VK_TAB), 1);
	h_eq_i64("Maj ne selectionne rien", desk_move(DESK_NONE, 3, 2, VK_SHIFT),
		DESK_NONE);
	h_eq_i64("liste vide", desk_move(0, 0, 2, VK_DOWN), DESK_NONE);
	h_eq_i64("zero ligne", desk_move(0, 3, 0, VK_DOWN), DESK_NONE);
}

int	main(void)
{
	h_begin("a20/desktop");
	h_run("bureau: grille d'un ecran normal", desktop_grille_ecran_normal);
	h_run("bureau: petites fenetres", desktop_petites_fenetres);
	h_run("bureau: test de clic", desktop_test_de_clic);
	h_run("bureau: clavier", desktop_clavier);
	return (h_end());
}
