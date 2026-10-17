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

static void	menu_clavier_et_anneau_de_focus(void)
{
	t_shell		sh;
	uint32_t	presents;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_key(&sh, sh.bar.id, VK_LWIN);
	h_eq_i64("pas de focus a l'ouverture", sh.sm.focus, SM_NO_ITEM);
	presents = fwm_find("Menu Démarrer")->presents;
	drv_key(&sh, sh.menu.id, VK_DOWN);
	h_eq_i64("bas: premiere tuile", sh.sm.focus, SM_IDX_RUN);
	h_true(fwm_find("Menu Démarrer")->presents > presents, "repeint");
	render_shot("shell_menu_focus");
	drv_key(&sh, sh.menu.id, VK_END);
	h_eq_i64("fin: Eteindre", sh.sm.focus, SM_IDX_SHUTDOWN);
	drv_key(&sh, sh.menu.id, VK_RETURN);
	h_eq_i64("entree ferme le menu", sh.sm.open, 0);
	sh_shutdown(&sh);
}

static void	menu_lancement_et_clics_neutres(void)
{
	t_shell	sh;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_click(&sh, sh.bar.id, rect_center(sh.reg.start));
	drv_click(&sh, sh.menu.id, rect_center(sh.sml.search));
	h_eq_i64("la recherche factice ne ferme pas", sh.sm.open, 1);
	h_eq_u64("aucun lancement", g_fos.nspawn, 0);
	drv_click(&sh, sh.menu.id, rect_center(sh.sml.tile[1]));
	h_eq_i64("menu ferme", sh.sm.open, 0);
	h_eq_u64("un lancement", g_fos.nspawn, 1);
	h_eq_str("programme lance", g_fos.path[0], "/system/bin/hello");
	h_eq_u64("sans privilege", g_fos.flags[0], 0);
	sh_shutdown(&sh);
	h_eq_u64("fils arretes a la fermeture de session", g_fos.killed, 1);
}

int	main(void)
{
	h_begin("a20/render-menu-kbd");
	h_run("rendu menu: clavier et anneau", menu_clavier_et_anneau_de_focus);
	h_run("rendu menu: lancement et clics neutres",
		menu_lancement_et_clics_neutres);
	return (h_end());
}
