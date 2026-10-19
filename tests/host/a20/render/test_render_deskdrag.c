#include "harness.h"
#include "velum/libk.h"
#include "velum/abi/abi_input.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static int32_t	maquette_x(int32_t slot, int32_t rows)
{
	return (24 + slot / rows * 90);
}

static int32_t	maquette_y(int32_t slot, int32_t rows)
{
	return (24 + slot % rows * 99);
}

static int32_t	maquette_rows(const t_shell *sh)
{
	return ((SCREEN_H - sh->lm.taskbar_h - 24 - 81) / 99 + 1);
}

static void	boot(t_shell *sh)
{
	render_reset(A20_FIXTURE);
	sh_boot(sh, 2, (char **)g_argv);
}

static void	glisser(t_shell *sh, t_point from, t_point to)
{
	t_uimsg	m;

	msg_button(&m, sh->desk.id, INP_MOUSE_DOWN, from);
	sh_dispatch(sh, &m);
	drv_move(sh, sh->desk.id, (t_point){(from.x + to.x) / 2,
		(from.y + to.y) / 2});
	drv_move(sh, sh->desk.id, to);
	msg_button(&m, sh->desk.id, INP_MOUSE_UP, to);
	sh_dispatch(sh, &m);
}

static void	deplacement_sous_le_seuil_ne_bouge_pas(void)
{
	t_shell	sh;
	t_rect	avant;
	t_point	c;

	boot(&sh);
	avant = sh.cells[0];
	c = rect_center(avant);
	glisser(&sh, c, (t_point){c.x + 3, c.y + 2});
	h_eq_i64("x inchange", sh.cells[0].x, avant.x);
	h_eq_i64("y inchange", sh.cells[0].y, avant.y);
	sh_shutdown(&sh);
}

static void	depose_sur_un_emplacement_libre(void)
{
	t_shell	sh;
	int32_t	rows;
	t_point	c;
	t_rect	autre;

	boot(&sh);
	rows = maquette_rows(&sh);
	autre = sh.cells[1];
	c = rect_center(sh.cells[0]);
	glisser(&sh, c, (t_point){maquette_x(4, rows) + 45,
		maquette_y(4, rows) + 40});
	h_eq_i64("x de l'emplacement 4", sh.cells[0].x, maquette_x(4, rows));
	h_eq_i64("y de l'emplacement 4", sh.cells[0].y, maquette_y(4, rows));
	h_eq_i64("largeur gardee", sh.cells[0].w, 90);
	h_eq_i64("hauteur gardee", sh.cells[0].h, 81);
	h_eq_i64("voisin x", sh.cells[1].x, autre.x);
	h_eq_i64("voisin y", sh.cells[1].y, autre.y);
	sh_shutdown(&sh);
}

static void	depose_sur_un_emplacement_occupe_echange(void)
{
	t_shell	sh;
	t_rect	a;
	t_rect	b;

	boot(&sh);
	a = sh.cells[0];
	b = sh.cells[2];
	glisser(&sh, rect_center(a), rect_center(b));
	h_eq_i64("0 prend x de 2", sh.cells[0].x, b.x);
	h_eq_i64("0 prend y de 2", sh.cells[0].y, b.y);
	h_eq_i64("2 prend x de 0", sh.cells[2].x, a.x);
	h_eq_i64("2 prend y de 0", sh.cells[2].y, a.y);
	sh_shutdown(&sh);
}

static void	icone_suit_le_curseur_pendant_le_glisser(void)
{
	t_shell	sh;
	t_uimsg	m;
	t_point	c;
	t_point	to;

	boot(&sh);
	c = rect_center(sh.cells[0]);
	to = (t_point){c.x + 100, c.y + 150};
	msg_button(&m, sh.desk.id, INP_MOUSE_DOWN, c);
	sh_dispatch(&sh, &m);
	drv_move(&sh, sh.desk.id, to);
	h_eq_i64("centre x sur le curseur", rect_center(sh.cells[0]).x, to.x);
	h_eq_i64("centre y sur le curseur", rect_center(sh.cells[0]).y, to.y);
	msg_button(&m, sh.desk.id, INP_MOUSE_UP, to);
	sh_dispatch(&sh, &m);
	sh_shutdown(&sh);
}

static void	glisser_ne_declenche_pas_le_double_clic(void)
{
	t_shell	sh;
	t_point	c;

	boot(&sh);
	c = rect_center(sh.cells[2]);
	glisser(&sh, c, (t_point){c.x + 200, c.y + 10});
	h_eq_i64("aucun lancement", g_fos.nspawn, 0);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-deskdrag");
	h_run("glisser: sous le seuil l'icone reste",
		deplacement_sous_le_seuil_ne_bouge_pas);
	h_run("glisser: depose sur un emplacement libre de la grille",
		depose_sur_un_emplacement_libre);
	h_run("glisser: emplacement occupe, echange",
		depose_sur_un_emplacement_occupe_echange);
	h_run("glisser: l'icone suit le curseur",
		icone_suit_le_curseur_pendant_le_glisser);
	h_run("glisser: pas de lancement", glisser_ne_declenche_pas_le_double_clic);
	return (h_end());
}
