#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "render.h"

static void	logon_demarrage_et_fenetre(void)
{
	t_logon	lg;

	render_reset(A20_FIXTURE);
	h_eq_i64("demarrage", logon_boot(&lg), 0);
	h_eq_u64("une fenetre", g_fwm.count, 1);
	h_eq_u64("plein ecran", fwm_find("Ouverture de session")->style,
		WS_FULLSCREEN);
	h_eq_u64("un compte", lg.nshown, 1);
	h_eq_u64("une tuile", lg.lay.count, 1);
	h_eq_i64("carte de 380 de large", lg.lay.card.w, 380);
	h_eq_i64("carte centree", lg.lay.card.x, (SCREEN_W - 380) / 2);
	h_eq_i64("carte a 52 sous la date", lg.lay.card.y, 311);
	render_shot("logon");
	uiwin_close(&lg.ui);
}

static void	logon_connexion_par_clic_sans_mot_de_passe(void)
{
	t_logon	lg;

	render_reset(A20_FIXTURE);
	logon_boot(&lg);
	lgd_click(&lg, lgd_tile(&lg, 0));
	h_eq_u64("le shell est lance", g_fos.nspawn, 1);
	h_eq_str("chemin du shell", g_fos.path[0], "/system/bin/shell");
	h_eq_str("nom du compte en argument", g_fos.arg[0], "Utilisateur");
	h_eq_u64("privileges de la session, ecriture a deleguer",
		g_fos.flags[0], PF_SPAWN | PF_POWER | PF_FSWRITE);
	h_eq_i64("retour a l'ecran de choix", lg.flow.mode, LM_PICK);
	h_eq_u64("fenetre reaffichee", fwm_find("Ouverture de session")->state,
		WSTATE_NORMAL);
	uiwin_close(&lg.ui);
}

static void	logon_extinction_avec_confirmation(void)
{
	t_logon	lg;

	render_reset(A20_FIXTURE);
	logon_boot(&lg);
	lgd_press_shutdown(&lg);
	h_eq_i64("confirmation demandee", lg.flow.mode, LM_SHUTDOWN);
	h_eq_i64("rien d'eteint avant confirmation", g_fos.power_op, -1);
	render_shot("logon_extinction");
	lgd_key(&lg, VK_ESCAPE);
	h_eq_i64("Echap annule", lg.flow.mode, LM_PICK);
	lgd_press_shutdown(&lg);
	lgd_key(&lg, VK_RETURN);
	h_eq_i64("Entree annule (focus par defaut sur Annuler)", lg.flow.mode,
		LM_PICK);
	h_eq_i64("toujours rien d'eteint", g_fos.power_op, -1);
	lgd_press_shutdown(&lg);
	lgd_key(&lg, VK_TAB);
	lgd_key(&lg, VK_RETURN);
	h_eq_i64("Tab puis Entree confirment", g_fos.power_op, POWER_OFF);
	uiwin_close(&lg.ui);
}

int	main(void)
{
	h_begin("a20/render-logon");
	h_run("rendu logon: demarrage", logon_demarrage_et_fenetre);
	h_run("rendu logon: connexion par clic",
		logon_connexion_par_clic_sans_mot_de_passe);
	h_run("rendu logon: extinction", logon_extinction_avec_confirmation);
	return (h_end());
}
