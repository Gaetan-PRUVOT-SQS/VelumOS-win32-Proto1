#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	dialogue_executer_lance_un_programme(void)
{
	t_shell	sh;
	t_fwin	*dlg;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_menu_item(&sh, SM_IDX_RUN);
	dlg = fwm_find("Exécuter");
	h_true(dlg != NULL, "dialogue ouvert");
	h_eq_u64("style de dialogue", dlg->style, WS_CAPTION | WS_SYSMENU);
	drv_type(&sh, dlg->id, "hello");
	render_shot("shell_executer");
	drv_key(&sh, dlg->id, VK_RETURN);
	h_eq_u64("un lancement", g_fos.nspawn, 1);
	h_eq_str("chemin resolu", g_fos.path[0], "/system/bin/hello");
	h_eq_i64("dialogue ferme", sh.dlg.ready, 0);
	h_eq_u64("fenetre detruite", fwm_find("Exécuter") == NULL, 1);
	sh_shutdown(&sh);
}

static void	dialogue_executer_refuse_un_nom_invalide(void)
{
	t_shell	sh;
	t_fwin	*dlg;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_menu_item(&sh, SM_IDX_RUN);
	dlg = fwm_find("Exécuter");
	drv_type(&sh, dlg->id, "../bad name");
	drv_key(&sh, dlg->id, VK_RETURN);
	h_eq_u64("rien lance", g_fos.nspawn, 0);
	h_eq_i64("boite de message", sh.dlg_kind, SH_DLG_NOTE);
	render_shot("shell_executer_erreur");
	drv_key(&sh, fwm_find("Exécuter")->id, VK_RETURN);
	h_eq_i64("message ferme", sh.dlg.ready, 0);
	drv_menu_item(&sh, SM_IDX_RUN);
	drv_key(&sh, fwm_find("Exécuter")->id, VK_RETURN);
	h_eq_u64("champ vide: rien lance", g_fos.nspawn, 0);
	h_eq_i64("champ vide: dialogue ferme", sh.dlg.ready, 0);
	sh_shutdown(&sh);
}

static void	dialogue_extinction_arreter_redemarrer_annuler(void)
{
	t_shell	sh;
	t_fwin	*dlg;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_menu_item(&sh, SM_IDX_SHUTDOWN);
	dlg = fwm_find("Éteindre l'ordinateur");
	h_true(dlg != NULL, "dialogue d'extinction");
	render_shot("shell_extinction");
	drv_key(&sh, dlg->id, VK_ESCAPE);
	h_eq_i64("annuler ferme", sh.dlg.ready, 0);
	h_eq_i64("rien eteint", g_fos.power_op, -1);
	drv_menu_item(&sh, SM_IDX_SHUTDOWN);
	drv_click(&sh, sh.dlg.win.id, rect_center(ctl_find(&sh.dlg.root,
				SH_ID_REBOOT)->rect));
	h_eq_i64("redemarrer", g_fos.power_op, POWER_REBOOT);
	drv_menu_item(&sh, SM_IDX_SHUTDOWN);
	drv_key(&sh, sh.dlg.win.id, VK_RETURN);
	h_eq_i64("arreter par defaut", g_fos.power_op, POWER_OFF);
	sh_shutdown(&sh);
}

static void	dialogue_fermer_la_session(void)
{
	t_shell	sh;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_menu_item(&sh, SM_IDX_LOGOFF);
	h_eq_i64("fin de session demandee", sh.quit, 1);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-dialogs");
	h_run("rendu dialogues: Executer", dialogue_executer_lance_un_programme);
	h_run("rendu dialogues: nom invalide",
		dialogue_executer_refuse_un_nom_invalide);
	h_run("rendu dialogues: extinction",
		dialogue_extinction_arreter_redemarrer_annuler);
	h_run("rendu dialogues: fermer la session", dialogue_fermer_la_session);
	return (h_end());
}
