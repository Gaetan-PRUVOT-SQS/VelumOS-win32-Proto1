#include "harness.h"
#include "velum/abi/abi_input.h"
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

static void	pw_effacement_apres_echap(void)
{
	t_logon	lg;
	t_ctl	*edit;

	render_reset(A20_FIXTURE_PW);
	logon_boot(&lg);
	lgd_click(&lg, lgd_tile(&lg, 0));
	lgd_type(&lg, "secret-42");
	edit = ctl_find(&lg.ui.root, LOGON_ID_PW);
	h_eq_str("saisie presente avant Echap", edit->text, "secret-42");
	lgd_key(&lg, VK_ESCAPE);
	h_true(all_zero(edit->text, CTL_TEXT_MAX), "tampon efface apres Echap");
	uiwin_close(&lg.ui);
}

static void	pw_effacement_apres_demande_extinction(void)
{
	t_logon	lg;
	t_ctl	*edit;

	render_reset(A20_FIXTURE_PW);
	logon_boot(&lg);
	lgd_click(&lg, lgd_tile(&lg, 0));
	lgd_type(&lg, "secret-42");
	lgd_press_shutdown(&lg);
	edit = ctl_find(&lg.ui.root, LOGON_ID_PW);
	h_eq_i64("confirmation demandee", lg.flow.mode, LM_SHUTDOWN);
	h_true(all_zero(edit->text, CTL_TEXT_MAX),
		"tampon efface quand l'extinction est demandee");
	uiwin_close(&lg.ui);
}

static void	pw_aucun_clair_dans_la_fenetre(void)
{
	t_logon	lg;
	t_ctl	*edit;

	render_reset(A20_FIXTURE_PW);
	logon_boot(&lg);
	lgd_click(&lg, lgd_tile(&lg, 0));
	lgd_type(&lg, "secret-42");
	edit = ctl_find(&lg.ui.root, LOGON_ID_PW);
	h_true(edit->flags & CTL_PASSWORD, "champ en mode mot de passe");
	h_true(lg.ui.root.focus == edit, "focus sur le champ masque");
	uiwin_close(&lg.ui);
}

int	main(void)
{
	h_begin("a20/render-logon-wipe");
	h_run("rendu logon: mot de passe efface apres Echap",
		pw_effacement_apres_echap);
	h_run("rendu logon: mot de passe efface avant la confirmation",
		pw_effacement_apres_demande_extinction);
	h_run("rendu logon: champ en mode masque", pw_aucun_clair_dans_la_fenetre);
	return (h_end());
}
