#include "harness.h"
#include "velum/libk.h"
#include "velum/abi/abi_input.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

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
	drv_move(sh, sh->desk.id, to);
	msg_button(&m, sh->desk.id, INP_MOUSE_UP, to);
	sh_dispatch(sh, &m);
}

static int	fond_nu(const t_shell *sh, t_rect c)
{
	t_point	p;

	p = rect_center(c);
	return (gfx_get(&sh->desk.surface, p)
		== sh->wall_px[p.y * sh->wall.w + p.x]);
}

static void	glisser_sur_la_corbeille_met_a_la_corbeille(void)
{
	t_shell	sh;
	t_rect	place;

	boot(&sh);
	place = sh.cells[2];
	h_true(!fond_nu(&sh, place), "l'icone est dessinee avant");
	glisser(&sh, rect_center(sh.cells[2]), rect_center(sh.cells[1]));
	h_eq_i64("un element a la corbeille", desk_trash_count(&sh.desk_state), 1);
	h_true(fond_nu(&sh, place), "l'emplacement est vide apres");
	h_eq_i64("clic dans le vide: rien",
		desk_pick(&sh.desk_state, sh.cells, sh.placed, rect_center(place).x,
			rect_center(place).y), DESK_NONE);
	sh_shutdown(&sh);
}

static void	entree_sur_la_corbeille_restaure(void)
{
	t_shell	sh;
	t_rect	place;

	boot(&sh);
	place = sh.cells[2];
	glisser(&sh, rect_center(sh.cells[2]), rect_center(sh.cells[1]));
	drv_click(&sh, sh.desk.id, rect_center(sh.cells[1]));
	drv_key(&sh, sh.desk.id, VK_RETURN);
	h_eq_i64("corbeille vide", desk_trash_count(&sh.desk_state), 0);
	h_true(!fond_nu(&sh, place), "l'icone est revenue a sa place");
	h_eq_i64("meme x", sh.cells[2].x, place.x);
	h_eq_i64("meme y", sh.cells[2].y, place.y);
	sh_shutdown(&sh);
}

static void	suppr_sur_la_corbeille_vide_definitivement(void)
{
	t_shell	sh;
	t_rect	place;

	boot(&sh);
	place = sh.cells[2];
	glisser(&sh, rect_center(sh.cells[2]), rect_center(sh.cells[1]));
	drv_click(&sh, sh.desk.id, rect_center(sh.cells[1]));
	drv_key(&sh, sh.desk.id, VK_DELETE);
	h_eq_i64("corbeille vide", desk_trash_count(&sh.desk_state), 0);
	drv_key(&sh, sh.desk.id, VK_RETURN);
	h_true(fond_nu(&sh, place), "rien a restaurer apres avoir vide");
	sh_shutdown(&sh);
}

static void	suppr_sur_une_icone_la_met_a_la_corbeille(void)
{
	t_shell	sh;

	boot(&sh);
	drv_click(&sh, sh.desk.id, rect_center(sh.cells[2]));
	drv_key(&sh, sh.desk.id, VK_DELETE);
	h_eq_i64("un element", desk_trash_count(&sh.desk_state), 1);
	sh_shutdown(&sh);
}

static void	ordinateur_et_corbeille_ne_se_suppriment_pas(void)
{
	t_shell	sh;

	boot(&sh);
	drv_click(&sh, sh.desk.id, rect_center(sh.cells[0]));
	drv_key(&sh, sh.desk.id, VK_DELETE);
	glisser(&sh, rect_center(sh.cells[0]), rect_center(sh.cells[1]));
	h_eq_i64("rien a la corbeille", desk_trash_count(&sh.desk_state), 0);
	h_true(desk_visible(&sh.desk_state, 0), "Ordinateur reste");
	h_true(desk_visible(&sh.desk_state, 1), "Corbeille reste");
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-desktrash");
	h_run("corbeille: glisser une icone dessus",
		glisser_sur_la_corbeille_met_a_la_corbeille);
	h_run("corbeille: Entree restaure", entree_sur_la_corbeille_restaure);
	h_run("corbeille: Suppr vide definitivement",
		suppr_sur_la_corbeille_vide_definitivement);
	h_run("corbeille: Suppr sur une icone",
		suppr_sur_une_icone_la_met_a_la_corbeille);
	h_run("corbeille: icones systeme protegees",
		ordinateur_et_corbeille_ne_se_suppriment_pas);
	return (h_end());
}
