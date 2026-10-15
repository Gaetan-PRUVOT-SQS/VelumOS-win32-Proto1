#include "harness.h"
#include "velum/luna.h"
#include "layout.h"
#include "logon_ink.h"

static void	close_to(const char *name, int32_t got, int32_t want)
{
	h_eq_i64(name, lay_clamp(got, want - LGI_TOL, want + LGI_TOL), got);
}

static void	check_head(t_logon *lg)
{
	int32_t	k[4];

	lgi_ref();
	lgi_scan(lg, rect_make(0, 60, SCREEN_W, 160), k);
	close_to("horloge: haut des chiffres a 116", k[LGI_TOP], 116);
	close_to("horloge: bas des chiffres a 203", k[LGI_BOTTOM], 203);
	close_to("horloge: centree", (k[LGI_LEFT] + k[LGI_RIGHT]) / 2,
		SCREEN_W / 2);
	lgi_scan(lg, rect_make(0, 224, SCREEN_W, 40), k);
	close_to("date: haut des lettres a 231", k[LGI_TOP], 231);
	close_to("date: base des lettres a 246", k[LGI_BOTTOM], 246);
	close_to("date: centree", (k[LGI_LEFT] + k[LGI_RIGHT]) / 2, SCREEN_W / 2);
}

static void	logon_en_tete_clair(void)
{
	t_logon	lg;

	render_reset(A20_FIXTURE);
	luna_theme_set(LUNA_THEME_LIGHT);
	logon_boot(&lg);
	check_head(&lg);
	uiwin_close(&lg.ui);
}

static void	logon_en_tete_sombre(void)
{
	t_logon	lg;

	render_reset(A20_FIXTURE);
	luna_theme_set(LUNA_THEME_DARK);
	logon_boot(&lg);
	check_head(&lg);
	render_shot("logon_sombre");
	uiwin_close(&lg.ui);
	luna_theme_set(LUNA_THEME_LIGHT);
}

int	main(void)
{
	h_begin("a20/render-logon-head");
	h_run("rendu logon: horloge et date, clair, encre de la maquette",
		logon_en_tete_clair);
	h_run("rendu logon: horloge et date, sombre, encre de la maquette",
		logon_en_tete_sombre);
	return (h_end());
}
