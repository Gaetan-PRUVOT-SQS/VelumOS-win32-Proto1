#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "render.h"

#define MENU_X 12
#define MENU_W 540
#define MENU_H 289
#define BAR_H 80

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	menu_geometrie_de_la_maquette(void)
{
	t_shell	sh;
	t_rect	r;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	h_eq_i64("barre de 80", sh.lm.taskbar_h, BAR_H);
	r = fwm_find("Menu Démarrer")->rect;
	h_eq_i64("x", r.x, MENU_X);
	h_eq_i64("bas a 80 du bord", r.y + r.h, (int32_t)sh.info.screen_h - BAR_H);
	h_eq_i64("largeur", r.w, MENU_W);
	h_eq_i64("hauteur", r.h, MENU_H);
	drv_click(&sh, sh.bar.id, rect_center(sh.reg.start));
	h_eq_i64("panneau local", sh.sml.panel.w, MENU_W);
	h_eq_i64("panneau local h", sh.sml.panel.h, MENU_H);
	h_eq_i64("exécuter et programmes", sh.sml.ntiles,
		(int32_t)(1 + sh.progs.count));
	sh_shutdown(&sh);
}

static void	menu_ouverture_survol_et_echap(void)
{
	t_shell	sh;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_move(&sh, sh.bar.id, rect_center(sh.reg.start));
	h_eq_i64("survol de Demarrer", sh.hot_start, 1);
	drv_click(&sh, sh.bar.id, rect_center(sh.reg.start));
	h_eq_i64("menu ouvert", sh.sm.open, 1);
	h_eq_u64("menu visible", fwm_find("Menu Démarrer")->state, WSTATE_NORMAL);
	h_eq_u64("capture au menu", g_fwm.captured, sh.menu.id);
	render_shot("shell_menu");
	drv_move(&sh, sh.menu.id, rect_center(sh.sml.tile[0]));
	h_eq_i64("survol de la premiere tuile", sh.sm.hot, 0);
	drv_move(&sh, sh.menu.id, rect_center(sh.sml.logoff));
	h_eq_i64("survol de Fermer la session", sh.sm.hot, SM_IDX_LOGOFF);
	drv_move(&sh, sh.menu.id, (t_point){-5, -5});
	h_eq_i64("survol dehors", sh.sm.hot, SM_NO_ITEM);
	drv_key(&sh, sh.menu.id, VK_ESCAPE);
	h_eq_i64("echap ferme", sh.sm.open, 0);
	h_eq_u64("menu cache", fwm_find("Menu Démarrer")->state, WSTATE_HIDDEN);
	h_eq_u64("capture relachee", g_fwm.captured, 0);
	sh_shutdown(&sh);
}

static void	menu_fermeture_exterieure_et_touche_windows(void)
{
	t_shell	sh;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_key(&sh, sh.bar.id, VK_LWIN);
	h_eq_i64("Windows ouvre", sh.sm.open, 1);
	drv_key(&sh, sh.bar.id, VK_LWIN);
	h_eq_i64("Windows referme", sh.sm.open, 0);
	drv_key(&sh, sh.bar.id, VK_RWIN);
	drv_click(&sh, sh.menu.id, (t_point){-20, -20});
	h_eq_i64("clic dehors ferme", sh.sm.open, 0);
	drv_click(&sh, sh.bar.id, rect_center(sh.reg.start));
	drv_click(&sh, sh.menu.id, (t_point){50, sh.sml.panel.h + 15});
	h_eq_i64("clic sur Demarrer ouvert referme", sh.sm.open, 0);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-menu");
	h_run("rendu menu: geometrie", menu_geometrie_de_la_maquette);
	h_run("rendu menu: ouverture, survol et Echap",
		menu_ouverture_survol_et_echap);
	h_run("rendu menu: fermetures",
		menu_fermeture_exterieure_et_touche_windows);
	return (h_end());
}
