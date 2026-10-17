#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "render.h"

static int	all_zero(const char *p, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if (p[i])
			return (0);
		i++;
	}
	return (1);
}

static void	pw_choix_du_compte_et_saisie(void)
{
	t_logon	lg;
	t_ctl	*edit;

	render_reset(A20_FIXTURE_PW);
	logon_boot(&lg);
	lgd_click(&lg, lgd_tile(&lg, 0));
	h_eq_i64("mode mot de passe", lg.flow.mode, LM_PASSWORD);
	edit = ctl_find(&lg.ui.root, LOGON_ID_PW);
	h_true(edit && (edit->flags & CTL_VISIBLE), "champ visible");
	h_true(lg.ui.root.focus == edit, "focus sur le champ");
	h_eq_u64("aucun lancement", g_fos.nspawn, 0);
	lgd_type(&lg, "abc");
	h_eq_str("saisie", edit->text, "abc");
	lgd_key(&lg, VK_ESCAPE);
	h_eq_i64("Echap revient au choix", lg.flow.mode, LM_PICK);
	h_eq_u64("compte oublie", lg.flow.selected, LF_NO_ACCOUNT);
	uiwin_close(&lg.ui);
}

static void	pw_echec_attente_puis_succes(void)
{
	t_logon	lg;
	t_ctl	*edit;

	render_reset(A20_FIXTURE_PW);
	logon_boot(&lg);
	lgd_click(&lg, lgd_tile(&lg, 0));
	lgd_type(&lg, "mauvais");
	lgd_key(&lg, VK_RETURN);
	edit = ctl_find(&lg.ui.root, LOGON_ID_PW);
	h_eq_i64("echec: message", lg.flow.msg, LMSG_BAD);
	h_true(all_zero(edit->text, sizeof(edit->text)), "mot de passe efface");
	h_true(g_fos.timer_armed > 0, "compte a rebours lance");
	lgd_type(&lg, "Passe 123");
	lgd_key(&lg, VK_RETURN);
	h_eq_i64("bon mot de passe pendant le delai: attente", lg.flow.msg,
		LMSG_WAIT);
	h_eq_u64("aucun lancement", g_fos.nspawn, 0);
	render_shot("logon_attente");
	g_fos.now += 1100000000ull;
	logon_tick(&lg);
	h_eq_i64("delai ecoule: message efface", lg.flow.msg, LMSG_NONE);
	uiwin_close(&lg.ui);
}

static void	pw_connexion_reussie(void)
{
	t_logon	lg;

	render_reset(A20_FIXTURE_PW);
	logon_boot(&lg);
	lgd_click(&lg, lgd_tile(&lg, 0));
	lgd_type(&lg, "Passe 123");
	render_shot("logon_mot_de_passe");
	lgd_key(&lg, VK_RETURN);
	h_eq_u64("shell lance", g_fos.nspawn, 1);
	h_eq_str("compte transmis", g_fos.arg[0], "Alice");
	h_eq_u64("sans PF_DISPLAY ni PF_INPUT", g_fos.flags[0] & (PF_DISPLAY
			| PF_INPUT), 0);
	h_eq_i64("retour au choix a la fermeture", lg.flow.mode, LM_PICK);
	h_true(all_zero(ctl_find(&lg.ui.root, LOGON_ID_PW)->text, CTL_TEXT_MAX),
		"mot de passe efface apres succes");
	uiwin_close(&lg.ui);
}

int	main(void)
{
	h_begin("a20/render-logon-pw");
	h_run("rendu logon mdp: choix et saisie", pw_choix_du_compte_et_saisie);
	h_run("rendu logon mdp: echec, attente", pw_echec_attente_puis_succes);
	h_run("rendu logon mdp: succes", pw_connexion_reussie);
	return (h_end());
}
