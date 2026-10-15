#include <stdint.h>
#include "harness.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	tick_at(t_shell *sh, uint64_t unix_s)
{
	g_fos.now = FOS_T0 + (unix_s - WALL_NS / 1000000000ull) * 1000000000ull;
	sh_clock_tick(sh);
}

static void	date_oracle_date_u(void)
{
	t_shell	sh;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	h_eq_str("date initiale", sh.date, "05/10/2026");
	h_eq_str("heure initiale", sh.clock, "14:07");
	tick_at(&sh, 1791301680ull);
	h_eq_str("06/10/2026 15:48", sh.date, "06/10/2026");
	h_eq_str("heure 15:48", sh.clock, "15:48");
	tick_at(&sh, 1798761540ull);
	h_eq_str("fin d'annee 31/12/2026 23:59", sh.date, "31/12/2026");
	tick_at(&sh, 1835395200ull);
	h_eq_str("jour bissextile 29/02/2028", sh.date, "29/02/2028");
	tick_at(&sh, 1861919940ull);
	h_eq_str("fin d'annee bissextile 31/12/2028", sh.date, "31/12/2028");
	tick_at(&sh, 4107542340ull);
	h_eq_str("2100 non bissextile: 28/02/2100", sh.date, "28/02/2100");
	tick_at(&sh, 4107542400ull);
	h_eq_str("2100 non bissextile: 01/03/2100", sh.date, "01/03/2100");
	sh_shutdown(&sh);
}

static void	date_barre_en_images(void)
{
	t_shell		sh;
	t_infospec	w;
	t_uimsg		m;

	render_reset(A20_FIXTURE);
	luna_theme_set(LUNA_THEME_LIGHT);
	sh_boot(&sh, 2, (char **)g_argv);
	w = (t_infospec){11, WS_DEFAULT, WSTATE_NORMAL, 1, "Bonjour"};
	msg_info(&m, WMS_WIN_ADD, &w);
	sh_dispatch(&sh, &m);
	tick_at(&sh, 1791301680ull);
	render_shot("barre_clair");
	luna_theme_set(LUNA_THEME_DARK);
	sh_bar_paint(&sh);
	render_shot("barre_sombre");
	luna_theme_set(LUNA_THEME_LIGHT);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-date");
	h_run("zone de notification: date, oracle date -u", date_oracle_date_u);
	h_run("barre: images claire et sombre", date_barre_en_images);
	return (h_end());
}
